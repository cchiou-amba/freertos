#include <idc.h>
#include <common.h>

extern int printf(const char *format, ...);

static uint32_t idc_base[] = {
	[I2C0] = IDC0_BASE,
	[I2C1] = IDC1_BASE,
	[I2C2] = IDC2_BASE,
	[I2C3] = IDC3_BASE,
	[I2C4] = IDC4_BASE,
	[I2C5] = IDC5_BASE,
	[I2C6] = IDC6_BASE,
	[I2C7] = IDC7_BASE,
	[I2C8] = IDC8_BASE,
	[I2CS] = IDCS_BASE,
#if defined(CMVI_SUPPORT_ONCHIP)
	[I2C_CMVI] = CMVI_IDC_BASE,
#endif
};

uint32_t idc_get_addr(uint8_t idc_id, uint32_t reg_offset)
{
	return idc_base[idc_id] + reg_offset;
}

uint32_t idc_readl(uint8_t idc_id, uint32_t reg_offset)
{
	return readl(idc_get_addr(idc_id, reg_offset));
}

void idc_writel(uint8_t idc_id, uint32_t reg_offset, uint32_t reg_val)
{
	writel(idc_get_addr(idc_id, reg_offset), reg_val);
}

/*
* IDC_clksrc=glck_axi/2 if pi_on else CM3-OSC/2 (16Mhz 10~22MHz)
* gclk_axi == CM3 run freq when PI_on
*/
void idc_clk_config(uint8_t idc_id, uint32_t idc_rate)
{
	uint32_t idc_prescale, duty_cycle, clksrc;

	if (!idc_rate)
		return;

	clksrc = current_cpufreq() / 2;
	duty_cycle = idc_readl(idc_id, IDC_DUTYCYCLE_OFFSET);

	idc_prescale =((clksrc / idc_rate)  - 2) / (4 + duty_cycle);
	idc_prescale--;

	idc_writel(idc_id, IDC_PSLL_OFFSET, idc_prescale & 0xff);
	idc_writel(idc_id, IDC_PSLH_OFFSET, (idc_prescale & 0xff00) >> 8);
}

void idc_wait_interrupt(uint8_t idc_id, uint32_t ctrl_start)
{
	uint32_t reg_ctrl;
	int wait_loop = 600000;

	idc_writel(idc_id, IDC_CTRL_OFFSET, ctrl_start);
	do {
		reg_ctrl = idc_readl(idc_id, IDC_CTRL_OFFSET);
		wait_loop--;
	} while (!(reg_ctrl & 0x00000002) && (wait_loop > 0));

	if (!wait_loop)
		printf("%s timeout\n", __func__);
}

void idc_set_8bit_addr(uint8_t idc_id, uint8_t addr, uint8_t rw)
{
	idc_writel(idc_id, IDC_DATA_OFFSET, addr | rw);
	idc_wait_interrupt(idc_id, 0x04);
}

void idc_stop(uint8_t idc_id)
{
	idc_writel(idc_id, IDC_CTRL_OFFSET, 0x08);
}

uint8_t idc_readb(uint8_t idc_id, uint32_t nack)
{
	idc_wait_interrupt(idc_id, nack);
	return (uint8_t)idc_readl(idc_id, IDC_DATA_OFFSET);
}

void idc_writeb(uint8_t idc_id, uint8_t data)
{
	idc_writel(idc_id, IDC_DATA_OFFSET, data);
	idc_wait_interrupt(idc_id, 0x00);
}

uint8_t idc_read_8_8(uint8_t idc_id, uint8_t adds, uint8_t sub_adds)
{
	uint8_t reg_val = 0;

	idc_set_8bit_addr(idc_id, adds, 0);
	idc_writeb(idc_id, sub_adds);
	idc_set_8bit_addr(idc_id, adds, 1);
	reg_val = idc_readb(idc_id, 0x1);
	idc_stop(idc_id);

	return reg_val;
}

void idc_write_8_8(uint8_t idc_id, uint8_t adds, uint8_t sub_adds, uint8_t reg_val)
{
	idc_set_8bit_addr(idc_id, adds, 0);
	idc_writeb(idc_id, sub_adds);
	idc_writeb(idc_id, reg_val);
	idc_stop(idc_id);
}
