/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <shell.h>
#include <gpio.h>
#include <spi.h>
#include <rpmsg.h>
#include <common.h>

int32_t strtou32(const char *str, uint32_t *out, char **endptr)
{
	const char *s = str;
	unsigned long acc;
	unsigned char c;
	unsigned long cutoff;
	int neg = 0, any, cutlim, base;
	int rval = 0;

	/*
	 * See strtol for comments as to the logic used.
	 */
	do {
		c = *s++;
	} while(isspace(c));

	if(c == '-') {
		neg = 1;
		c = *s++;
	} else if(c == '+') {
		c = *s++;
	}

	if(c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	} else {
		base = 10;
	}

	cutoff = (unsigned long)UINT_MAX / (unsigned long)base;
	cutlim = (unsigned long)UINT_MAX % (unsigned long)base;

	for(acc = 0, any = 0;; c = *s++) {
		if(c == '\0') {
			break;
		}

		if(!isascii(c)) {
			rval = -1;
			break;
		}

		if(isdigit(c)) {
			c -= '0';
		} else if(isalpha(c)) {
			c -= isupper(c) ? 'A' - 10 : 'a' - 10;
		} else {
			rval = -1;
			break;
		}

		if(c >= base) {
			rval = -1;
			break;
		}

		if(any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
			any = -1;
		} else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}

	if(any < 0) {
		acc = UINT_MAX;
	} else if(neg) {
		acc = -acc;
	}

	if(out != NULL) {
		*out = any ? acc : *str;
	}

	if(endptr != NULL) {
		*endptr = (char *)s - 1;
	}

	return rval;
}

int32_t cmd_r32(int argc, char **argv)
{
	int ret = 0, i;
	uint32_t addr, size = 4;

	if(argc < 2) {
		printf("r32 addr [size] \n");
		return -1;
	} else {
		ret = strtou32(argv[1], &addr, NULL);

		if(ret < 0) {
			return ret;
		}

		if(argc > 2) {
			ret = strtou32(argv[2], &size, NULL);

			if(ret < 0) {
				return ret;
			}
		}

		for(i = 0; i < size; i += 4) {
			if((i % 16) == 0) {
				printf("%s0x%08x:", i ? "\n" : "", addr);
			}

			__DSB();
			printf("    0x%08x", readl(addr));
			addr += 4;
		}

		printf("\n");
	}

	return 0;
}
CMD_EXPORT_ALIAS(cmd_r32, r32, r32 addr [size (default 4)]);

int cmd_w32(int argc, char **argv)
{
	int ret = 0;
	uint32_t addr, value;

	if(argc < 3) {
		printf("usage: w32 addr value\n");
		return -1;
	} else {
		ret = strtou32(argv[1], &addr, NULL);

		if(ret < 0) {
			return ret;
		}

		ret = strtou32(argv[2], &value, NULL);

		if(ret < 0) {
			return ret;
		}

		writel(addr, value);
		__DSB();
	}

	return 0;
}
CMD_EXPORT_ALIAS(cmd_w32, w32, w32 addr value);

#if defined(ARCH_CM3)
int board_send_ipi(uint32_t mask)
{
	uint32_t umsk = mask & 3;
	uint32_t data = 0;

	data = readl(CM3_SCRATCHPAD_BASE);
	data &= ~umsk;
	writel(CM3_SCRATCHPAD_BASE, data);
	__DSB();
	writel(CM3_SCRATCHPAD_BASE, umsk);
	return 0;
}

int cmd_ipi(int argc, char **argv)
{
	return board_send_ipi(3);
}
CMD_EXPORT_ALIAS(cmd_ipi, ipi, send ipi to cpu);
#endif

/*===========================================================================*/
static void cmd_gpio_set(int gpio, int on)
{
	gpio_config_sw_out(gpio);

	if(on) {
		gpio_set(gpio);
	} else {
		gpio_clr(gpio);
	}
}

static void cmd_gpio_get(uint32_t gpio)
{
	uint32_t gpio_val;

	gpio_config_sw_in(gpio);
	gpio_val = gpio_get(gpio);
	printf("gpio[%d] = %d\n", gpio, gpio_val);
}

static void cmd_gpio_hw(int gpio, int alt_func)
{
	gpio_config_hw(gpio, alt_func);
}

/*===========================================================================*/
static int cmd_gpio(int argc, char *argv[])
{
	uint32_t gpio_id = 0xFFFFFFFF;

	if(argc < 3) {
		return -1;
	}

	strtou32(argv[2], &gpio_id, NULL);

	if(strcmp(argv[1], "get") == 0) {
		cmd_gpio_get(gpio_id);
	} else if(strcmp(argv[1], "set") == 0) {
		cmd_gpio_set(gpio_id, 1);
	} else if(strcmp(argv[1], "clr") == 0) {
		cmd_gpio_set(gpio_id, 0);
	} else if(strcmp(argv[1], "hw") == 0) {
		uint32_t alt_func = 1;

		if(argc >= 4) {
			strtou32(argv[3], &alt_func, NULL);
		}

		cmd_gpio_hw(gpio_id, alt_func);
	}

	return 0;
}

CMD_EXPORT_ALIAS(cmd_gpio, gpio, gpio [set | clr | get] id or gpio [hw] id func);
/*===========================================================================*/

static void cmd_type(int argc, char **argv)
{
	printf("sizeof(uint16_t) = %u bytes \n", sizeof(uint16_t));
	printf("sizeof(uint32_t) = %u bytes \n", sizeof(uint32_t));
	printf("sizeof(uint64_t) = %u bytes \n", sizeof(uint64_t));
	printf("sizeof(long) = %u bytes \n", sizeof(long));
}
CMD_EXPORT_ALIAS(cmd_type, type, show types width);

/*===========================================================================*/

#if defined(CONFIG_CM3_SPI)
static void buff_fill(uint8_t *buf, uint32_t len)
{
	unsigned int i, chksum = 0;

	if (len == 1) {
		*(unsigned short *)buf = 0x12;
	} else if (len == 2) {
		*(unsigned short *)buf = 0x1234;
	} else {
		for (i = 0; i < len - 2; i++) {
			buf[i] = (i + 1) & 0xff;
			chksum += buf[i];
		}

		buf[len - 2] = chksum & 0xff; 		/* low bytes */
		buf[len - 1] = (chksum >> 8) & 0xff;	/* high bytes */
	}

	printf("fill data [%u] cheksum: 0x%x\n", len, chksum);
}

static int buff_check(uint8_t *buf, uint32_t len)
{
	unsigned int i, chksum = 0,  calsum = 0;

	if (len == 1) {
		switch (*(unsigned char *)buf) {
		case 0x00:
			return 0;
		case 0x12:
			return 1;
		default:
			return -1;
		}
	} else if (len == 2) {
		switch (*(unsigned short *)buf) {
		case 0x0000:
			return 0;
		case 0x1234:
			return 1;
		default:
			return -1;
		}
	} else {
		for (i = 0; i < len - 2; i++)
			calsum += buf[i];

		if (calsum == 0)
			return 0;

		calsum &= 0xffff;
		chksum = buf[len-2] + (buf[len-1] << 8);

		return (chksum != calsum)? -1 : 1;
	}
}

static void buff_show(uint8_t *buf, uint32_t len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != 0)
			break;
	}

	if (i == len) {
		printf("---all zero---\n");
		return;
	}

	for (i = 0; i < len; i++) {
		if (i && ((i & 7) == 0))
			printf("\n");
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
}

static uint8_t tx_buf[1024] = {0};
static uint8_t rx_buf[1024] = {0};
volatile uint32_t xfer_state = 0;

void spi_xfer_cplt(spi_t *spi)
{
	xfer_state = 1;
}

static int cmd_spi(const int argc, char **argv)
{
	uint32_t len = 8;
	int i = 0;
	char dev_name[16] = {0};
	spi_t *spi = NULL;

	if (argc < 2) {
		return -1;
	}

	strncpy(dev_name, argv[argc - 1], sizeof(dev_name));
	spi = spi_dev(dev_name);
	if (spi == NULL) {
		printf("%s not found!\n", dev_name);
		return -1;
	}

	for (i = 1; i < argc - 1; i++) {
		if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--len")) {
			strtou32(argv[++i], &len, NULL);
		} else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bpw")) {
			++i; /* == 8 or > 8(16) */
		}
	}

	buff_fill(tx_buf, len);
	memset(rx_buf, 0xff, len);

#ifdef CONFIG_CM3_SPI_POLLING_MODE
	switch (spi_xfer_poll(spi, tx_buf, rx_buf, len, 5000)) {
	case 0:
		break;

	default:
		printf("xfer err!\n");
		break;
	}
#else /* Interrupt */
	TickType_t tick;

	tick = xTaskGetTickCount();
	xfer_state = 0;

	if (spi_xfer_intr(spi, tx_buf, rx_buf, len & 0xffff)) {
		printf("xfer err!\n");
	}

	while (xfer_state == 0) {
		if (xTaskGetTickCount() - tick > pdMS_TO_TICKS(5000)) {
			printf("xfer timeout!\n");
			break;
		}
	}
#endif
	if (buff_check(rx_buf, len) <= 0) {
		printf("recv chk error\n");
		buff_show(rx_buf, len);
	}

	return 0;
}
CMD_EXPORT_ALIAS(cmd_spi, spi, spi [-l length] dev_name);
#endif

#if 0
/* timer test case */
#define BIT_MASK	0x2000000
#define GPIO_PIN	25

void timer0_intr1(void)
{
	static uint32_t value = BIT_MASK;
	value ^= BIT_MASK;
	writel(GPIO0_BASE + GPIO_DATA_OFFSET, value);
}

static int cmd_timer(int argc, char *argv[])
{
	uint32_t g_ticks = 0;
	uint32_t n_usec, time_ctrl, ticks_per_us_apb, adjust = 0;

	ticks_per_us_apb = current_apbfreq() / 1000 / 1000;
	if (argc > 1)
		strtou32(argv[1], &n_usec, NULL);
	if (argc > 2)
		strtou32(argv[2], &adjust, NULL);

	g_ticks = n_usec * ticks_per_us_apb;
	printf("Apb_clk:%u  cycle_us:%u and cycle_ticks:%u adjust=%u\n",
			current_apbfreq(),n_usec, g_ticks, adjust);

	gpio_config_sw_out(GPIO_PIN);

	writel(TIMER0_BASE + TIMER0_STATUS_OFFSET, g_ticks);  /* anyvalue is good */
	writel(TIMER0_BASE + TIMER0_RELOAD_OFFSET, g_ticks);
	writel(TIMER0_BASE + TIMER0_MATCH1_OFFSET, (g_ticks/2));
	writel(TIMER0_BASE + TIMER0_MATCH2_OFFSET, 0);
	time_ctrl = readl(TIMER0_BASE + TIMER_CTRL_OFFSET);
	time_ctrl = 0x1;
	writel(TIMER0_BASE + TIMER_CTRL_OFFSET, time_ctrl);

	return 0;
}
CMD_EXPORT_ALIAS(cmd_timer, timer, timer n_us);

void timer0_run_test(uint32_t n_usec)
{
	uint32_t g_ticks = 0;
	uint32_t time_ctrl, ticks_per_us_apb;

	ticks_per_us_apb = current_apbfreq() / 1000 / 1000;
	g_ticks = n_usec * ticks_per_us_apb;

	printf("Apb_clk:%u  cycle_us:%u and cycle_ticks:%u \n",
			current_apbfreq(),n_usec, g_ticks);

	gpio_config_sw_out(GPIO_PIN);

	writel(TIMER0_BASE + TIMER0_STATUS_OFFSET, g_ticks);
	writel(TIMER0_BASE + TIMER0_RELOAD_OFFSET, g_ticks);
	writel(TIMER0_BASE + TIMER0_MATCH1_OFFSET, (g_ticks/2));
	writel(TIMER0_BASE + TIMER0_MATCH2_OFFSET, 0);
	time_ctrl = readl(TIMER0_BASE + TIMER_CTRL_OFFSET);
	time_ctrl = 0x1;
	writel(TIMER0_BASE + TIMER_CTRL_OFFSET, time_ctrl);
}
#endif
