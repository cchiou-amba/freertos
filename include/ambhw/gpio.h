 /*
 * Copyright (c) 2026 Ambarella International LP
 *
 * License Identifier: AMBARELLA-2-Clause
 */
#ifndef __AMBHW_GPIO_H__
#define __AMBHW_GPIO_H__

#include <plat.h>

#define GPIO_MAX_LINE			12
#define GPIO_LINE_OF_BANK		32

#define GPIO_DATA_OFFSET		0x00
#define GPIO_DIR_OFFSET			0x04
#define GPIO_IS_OFFSET			0x08
#define GPIO_IBE_OFFSET			0x0c
#define GPIO_IEV_OFFSET			0x10
#define GPIO_IE_OFFSET			0x14
#define GPIO_AFSEL_OFFSET		0x18
#define GPIO_RIS_OFFSET			0x1c
#define GPIO_MIS_OFFSET			0x20
#define GPIO_IC_OFFSET			0x24
#define GPIO_MASK_OFFSET		0x28
#define GPIO_ENABLE_OFFSET		0x2c

#define GPIO_BASE(x)			((x) == 0 ? GPIO0_BASE : \
					 ((x) == 1 ? GPIO1_BASE : \
					  ((x) == 2 ? GPIO2_BASE : \
					   GPIO3_BASE)))

#define GPIO_DATA_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_DATA_OFFSET)
#define GPIO_DIR_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_DIR_OFFSET)
#define GPIO_IS_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_IS_OFFSET)
#define GPIO_IBE_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_IBE_OFFSET)
#define GPIO_IEV_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_IEV_OFFSET)
#define GPIO_IE_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_IE_OFFSET)
#define GPIO_AFSEL_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_AFSEL_OFFSET)
#define GPIO_RIS_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_RIS_OFFSET)
#define GPIO_MIS_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_MIS_OFFSET)
#define GPIO_IC_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_IC_OFFSET)
#define GPIO_MASK_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_MASK_OFFSET)
#define GPIO_ENABLE_REG(gpio)		(GPIO_BASE((gpio) / GPIO_LINE_OF_BANK) + GPIO_ENABLE_OFFSET)

#endif
