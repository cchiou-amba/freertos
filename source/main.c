/*******************************************************************************
 *
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 *
 ******************************************************************************/
#include <plat.h>
#include <common.h>
#include <task.h>

void __attribute__((weak)) board_hw_init(void) {}
void __attribute__((weak)) shell_task_init(void) {}
void __attribute__((weak)) app_task_init(void) {}
void __attribute__((weak)) app1_task_init(void) {}
void __attribute__((weak)) app2_task_init(void) {}

static void boot_message(void)
{
	printf("\n FreeRTOS Runs on CPU_%s_C%u "
		"cpufreq:%uHz "
		"apbfreq:%uHz "
		"ahbfreq:%uHz\n",
		ARCH_NAME,
		current_cpuid(),
		current_cpufreq(),
		current_apbfreq(),
		current_ahbfreq());
}

static void boot_crashed(void)
{
	printf("FreeRTOS crashed \n");
	__asm__ __volatile__("b	 .");
}

#if defined(CONFIG_BOOT_R52)
#include <i2c.h>
#include <ambhw/gpio.h>
#include <ambhw/spi.h>

#define EEPROM_ADDR			   (0x54)
#define EEPROM_READ_SIZE       (2048)
#define EEPROM_DATA_BASE_ADDR  (0x40000000)
#define EEPROM_WRITE_SUCCESS   (0x12345678)
#define EEPROM_WRITE_FAILED    (0x0)
#define TASK_EEPROM_STACK_SIZE (128 + EEPROM_READ_SIZE)
#define TASK_EEPROM_PRI		   (tskIDLE_PRIORITY + 4)

static StackType_t task_eeprom_stack[TASK_EEPROM_STACK_SIZE] __attribute__((aligned(8)));
static StaticTask_t tasktcb_eeprom;

extern void virtio_i2c_init(int id);
extern void virtio_gpio_init(void);
extern void virtio_spi_init(int id);

static void eeprom_task_entry(void *arg)
{
	int rval = 0;
	uint8_t read_addr[2] = {0x00,0x00};
	uint8_t read_data[EEPROM_READ_SIZE];
	struct i2c_msg msg[2];
	msg[0].buf = read_addr;
	msg[0].len = 2;
	msg[1].buf = read_data;
	msg[1].len = EEPROM_READ_SIZE;
	rval = i2c_read(I2C2,EEPROM_ADDR,msg);
	uint32_t *data32 = (uint32_t *)read_data;

	if (rval != 0) {
		printf("i2c read failed\n");
		writel(EEPROM_DATA_BASE_ADDR + EEPROM_READ_SIZE, EEPROM_WRITE_FAILED);
		goto END;
	}

	for (int i = 0; i < EEPROM_READ_SIZE / 4; i++) {
		writel(EEPROM_DATA_BASE_ADDR + i * 4, data32[i]);
		__DSB();
	}

	writel(EEPROM_DATA_BASE_ADDR + EEPROM_READ_SIZE, EEPROM_WRITE_SUCCESS);

END:
	printf("status:%08x\n", readl(EEPROM_DATA_BASE_ADDR + EEPROM_READ_SIZE));
	vTaskDelete(NULL);
}

static void eeprom_task_init(void)
{
	TaskHandle_t taskhandler;
	taskhandler = xTaskCreateStatic(
		eeprom_task_entry,
		"eeprom",
		TASK_EEPROM_STACK_SIZE,
		NULL,
		TASK_EEPROM_PRI,
		task_eeprom_stack,
		&tasktcb_eeprom);
}

void r52_hw_init(void)
{
	i2c_init(I2C2, 100000, 1);
	spi_init(SPI1);
	eeprom_task_init();
	virtio_gpio_init();
	virtio_i2c_init(I2C2);
	virtio_spi_init(SPI1);
}
#endif

int main(void)
{
#if defined(CONFIG_BOOT_R52)
	r52_hw_init();
#endif
	board_hw_init();
	boot_message();

	/* tasks init */
	shell_task_init();
	app_task_init();
	app1_task_init();
	app2_task_init();

	/* start the tasks */
	vTaskStartScheduler();

	/* Should Not Reach Here */
	boot_crashed();

	return 0;
}
