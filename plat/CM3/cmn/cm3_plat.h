/*******************************************************************************
 *
 * Copyright (c) 2023 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella International LP or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella International LP
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA INTERNATIONAL LP OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/


#ifndef __AMBA_CM3_PLAT_H__
#define __AMBA_CM3_PLAT_H__

#define CV72		(32000)
#define CV3AD685	(33000)
#define CV75		(34000)
#define N1_655		(35000)

#if defined(CONFIG_CHIP_CV72)
#define CHIP_REV	CV72
#elif defined(CONFIG_CHIP_CV3AD685)
#define CHIP_REV	CV3AD685
#elif defined(CONFIG_CHIP_CV75)
#define CHIP_REV	CV75
#elif defined(CONFIG_CHIP_N1_655)
#define CHIP_REV	N1_655
#else
#error "Undefined CHIP_REV"
#endif

#define INVLIAD_ADDRESS		0xFFFFFFF0

#define BOOT_SPACE_BASE		0x00000000
#define BOOT_SPACE_SIZE		0x8000
#define CPUID_BASE		0x50000000

#if (CHIP_REV == CV72) || (CHIP_REV == CV75)
#define CM3CORE_INSTANCES	1
#define CM3CORE_CLUSTERID	INVLIAD_ADDRESS
#elif (CHIP_REV == CV3AD685)
#define CM3CORE_INSTANCES	2
#define CM3CORE_CLUSTERID	0x54020020
#elif (CHIP_REV == N1_655)
#define CM3CORE_INSTANCES	1
#define CM3CORE_CLUSTERID	0x54020020
#endif

#define SSPI_BASE		0x50010000
#define SPI0_BASE		0X50011000
#define SPI1_BASE		0x50012000
#define SPI2_BASE		0x50013000
#define SPI3_BASE		0x50014000

#if (CHIP_REV == CV72) || (CHIP_REV == CV75) || (CHIP_REV == N1_655)
#define SPI4_BASE		(INVLIAD_ADDRESS + 1)
#define SPI5_BASE		(INVLIAD_ADDRESS + 2)
#define SPI6_BASE		(INVLIAD_ADDRESS + 3)
#define SPI7_BASE		(INVLIAD_ADDRESS + 4)
#elif (CHIP_REV == CV3AD685)
#define SPI4_BASE		0x50025000
#define SPI5_BASE		0x50026000
#define SPI6_BASE		0x5002C000
#define SPI7_BASE		0x5002D000
#endif

#define APB_UART_BASE		0x54000000
#define AHB_UART0_BASE		0x50017000
#define AHB_UART1_BASE		0x50018000
#define AHB_UART2_BASE		0x50019000
#define AHB_UART3_BASE		0x5001A000
#define AHB_UART4_BASE		(INVLIAD_ADDRESS + 1)
#define AHB_UART5_BASE		(INVLIAD_ADDRESS + 2)
#define AHB_UART6_BASE		(INVLIAD_ADDRESS + 3)
#define MAX_VALID_AHB_UART_PORT	5

#if (CHIP_REV == CV72)
#define TCC_CTRL_BASE		0x50024000
#define I3C_BASE		0x54011000
#elif (CHIP_REV == CV3AD685) || (CHIP_REV == CV75) || (CHIP_REV == N1_655)
#define TCC_CTRL_BASE		INVLIAD_ADDRESS
#define I3C_BASE		INVLIAD_ADDRESS
#endif

#define IRIF_BASE		0x54001000
#if (CHIP_REV == N1_655)
#define ADC_BASE		INVLIAD_ADDRESS
#else
#define ADC_BASE		0x54002000
#endif

#define PWM0_BASE		0x54003000
#define PWM1_BASE		0x5400C000
#define PWM2_BASE		0x5400D000

#define TIMER0_BASE		0x54004000
#define TIMER1_BASE		0x54005000
#define TIMER2_BASE		0x54006000

#define TIMER0_STATUS_OFFSET	0x00
#define TIMER0_RELOAD_OFFSET	0x04
#define TIMER0_MATCH1_OFFSET	0x08
#define TIMER0_MATCH2_OFFSET	0x0c
#define TIMER1_STATUS_OFFSET	0x10
#define TIMER1_RELOAD_OFFSET	0x14
#define TIMER1_MATCH1_OFFSET	0x18
#define TIMER1_MATCH2_OFFSET	0x1c
#define TIMER2_STATUS_OFFSET	0x20
#define TIMER2_RELOAD_OFFSET	0x24
#define TIMER2_MATCH1_OFFSET	0x28
#define TIMER2_MATCH2_OFFSET	0x2c
#define TIMER_CTRL_OFFSET	0x30

#define IDCS_BASE		0x54007000
#define IDC0_BASE		0x54008000
#define IDC1_BASE		0x54009000
#define IDC2_BASE		0X5400A000
#define IDC3_BASE		0x5400B000
#if (CHIP_REV == CV72) || (CHIP_REV == CV75) || (CHIP_REV == N1_655)
#define IDC4_BASE		(INVLIAD_ADDRESS + 1)
#define IDC5_BASE		(INVLIAD_ADDRESS + 2)
#define IDC6_BASE		(INVLIAD_ADDRESS + 3)
#define IDC7_BASE		(INVLIAD_ADDRESS + 4)
#define IDC8_BASE		(INVLIAD_ADDRESS + 5)
#elif (CHIP_REV == CV3AD685)
#define IDC4_BASE		0x5400E000
#define IDC5_BASE		0x5400F000
#define IDC6_BASE		0x5401C000
#define IDC7_BASE		0x5401D000
#define IDC8_BASE		0x5401E000
#endif

#define IOMUX_BASE		0x54010000

#if (CHIP_REV == CV72) || (CHIP_REV == CV3AD685) || (CHIP_REV == N1_655)
#define SDIO_IOMUX		INVLIAD_ADDRESS
#elif (CHIP_REV == CV75)
#define SDIO_IOMUX		0x54011000
#endif

#if (CHIP_REV == CV72) || (CHIP_REV == CV75) || (CHIP_REV == N1_655)
#define WDT_CM3C1_BASE		INVLIAD_ADDRESS
#elif (CHIP_REV == CV3AD685)
#define WDT_CM3C1_BASE		0x54029000
#endif

#if (CHIP_REV == CV72)
#define WDT_CM3C0_BASE		0x54019000
#elif (CHIP_REV == CV3AD685) || (CHIP_REV == N1_655)
#define WDT_CM3C0_BASE		0x54028000
#elif (CHIP_REV == CV75)
#define WDT_CM3C0_BASE		CMVI_WDT_BASE
#endif

#define WDTC0_REG(x)		(WDT_CM3C0_BASE + (x))
#define WDTC1_REG(x)		(WDT_CM3C1_BASE + (x))

#if (CHIP_REV == CV72) || (CHIP_REV == CV3AD685) || (CHIP_REV == N1_655)
#define SCRATCHPAD_NSEC_BASE	0x5003F000
#define CM3_SCRATCHPAD_BASE	0x54020000
#elif (CHIP_REV == CV75)
#define SCRATCHPAD_NSEC_BASE	0x5004F000
#define CM3_SCRATCHPAD_BASE	0x58000000
#endif
#define CM3_SCRATCHPAD_REG(x)	(CM3_SCRATCHPAD_BASE + (x))
#define CM3_NSCRATCHPAD_REG(x)	(SCRATCHPAD_NSEC_BASE + (x))

#define SCRATCHPAD_DATA0_OFFSET	0x6c
#define SCRATCHPAD_DATA1_OFFSET	0x70
#define SCRATCHPAD_DATA2_OFFSET	0x74
#define SCRATCHPAD_DATA3_OFFSET	0x78

#if (CHIP_REV == CV75)
#define CMVI_SUPPORT_ONCHIP
#define CMVI_IRAM_BASE			0x00010000 /* internal 64KB SRAM */
#define CMVI_CM3_SSI_BASE		0x58001000
#define CMVI_IDC_BASE			0x58100000
#define CMVI_WDT_BASE			0x58101000
#define CMVI_TIMER_BASE			0x58102000
#define CMVI_GPIO_BASE			0x58103000
#define CMVI_IOMUX_BASE			0x58104000
#define IOMUX0_OFFSET			(0x0)
#define CMVI_IOMUX_REG(x)		(CMVI_IOMUX_BASE + (x))
#define CMVI_GPIO_REG(x)		(CMVI_GPIO_BASE + (x))
/* resuse CM3_NSCRATCHPAD_REG(DATA3) to pass SRS confirm data (by ATF) */
#define CMVI_SRS_MARK			0x5253  /* 'SR' */
#define CMVI_PWR_TEST_MARK		0x5053  /* 'PR' */
#define CMVI_SRS_REG			CM3_NSCRATCHPAD_REG(SCRATCHPAD_DATA3_OFFSET)
#define CMVI_DATA0_REG			CM3_NSCRATCHPAD_REG(SCRATCHPAD_DATA0_OFFSET)
#define CMVI_DATA1_REG			CM3_NSCRATCHPAD_REG(SCRATCHPAD_DATA1_OFFSET)
#define CMVI_DATA2_REG			CM3_NSCRATCHPAD_REG(SCRATCHPAD_DATA2_OFFSET)
#endif

/* GPIO_BASE also include CMVI_GPIO_BASE if support */
#define GPIO0_BASE		0x54013000
#define GPIO1_BASE		0x54014000
#define GPIO2_BASE		0x54015000
#define GPIO3_BASE		0x54016000
#if (CHIP_REV == CV72) || (CHIP_REV == CV75) || (CHIP_REV == N1_655)
#define GPIO4_BASE		(INVLIAD_ADDRESS + 1)
#define GPIO5_BASE		(INVLIAD_ADDRESS + 2)
#define GPIO6_BASE		(INVLIAD_ADDRESS + 3)
#if (CHIP_REV == CV72)	|| (CHIP_REV == N1_655)
#define GPIO_INSTANCES		4
#elif (CHIP_REV == CV75)
#define GPIO_INSTANCES		11				/* PIN320 maps to CMVI_PIN0 */
#endif
#elif (CHIP_REV == CV3AD685)
#define GPIO4_BASE		0x54017000
#define GPIO5_BASE		0x54018000
#define GPIO6_BASE		0x54019000
#define GPIO_INSTANCES		7
#endif
#define GPIO7_BASE		(INVLIAD_ADDRESS + 7)
#define GPIO8_BASE		(INVLIAD_ADDRESS + 8)
#define GPIO9_BASE		(INVLIAD_ADDRESS + 9)
#if (CHIP_REV == CV72) || (CHIP_REV == CV3AD685) || (CHIP_REV == N1_655)
#define GPIO10_BASE		(INVLIAD_ADDRESS + 10)
#elif (CHIP_REV == CV75)
#define GPIO10_BASE		(CMVI_GPIO_BASE)		/* maps to CMVI_GPIO */
#endif

/* CM3 boot space size 32KB */
#define CM3_DEFAULT_BOOT_SPCSZ	0x8000
/* CM3 remap external DDR memory start address */
#define CM3_DRAM_ADDRESS	0x60000000

#ifndef __ASM__

#include <stdint.h>
#include <hwio.h>

/* -------------------------  Interrupt Number Definition  ------------------------ */
typedef enum IRQn {
	/* -------------------  Processor Exceptions Numbers  ----------------------------- */
	NonMaskableInt_IRQn           = -14,     /*  2 Non Maskable Interrupt */
	HardFault_IRQn                = -13,     /*  3 HardFault Interrupt */
	MemoryManagement_IRQn         = -12,     /*  4 Memory Management Interrupt */
	BusFault_IRQn                 = -11,     /*  5 Bus Fault Interrupt */
	UsageFault_IRQn               = -10,     /*  6 Usage Fault Interrupt */
	SVCall_IRQn                   =  -5,     /* 11 SV Call Interrupt */
	DebugMonitor_IRQn             =  -4,     /* 12 Debug Monitor Interrupt */
	PendSV_IRQn                   =  -2,     /* 14 Pend SV Interrupt */
	SysTick_IRQn                  =  -1,     /* 15 System Tick Interrupt */

	/* -------------------  Processor Interrupt Numbers  ------------------------------ */
	Interrupt0_IRQn               =   0,
	Interrupt1_IRQn               =   1,
	Interrupt2_IRQn               =   2,
	Interrupt3_IRQn               =   3,
	Interrupt4_IRQn               =   4,
	Interrupt5_IRQn               =   5,
	Interrupt6_IRQn               =   6,
	Interrupt7_IRQn               =   7,
	Interrupt8_IRQn               =   8,
	Interrupt9_IRQn               =   9,

	/* 10-19 */
	Interrupt10_IRQn               =   10,
	Interrupt11_IRQn               =   11,
	Interrupt12_IRQn               =   12,
	Interrupt13_IRQn               =   13,
	Interrupt14_IRQn               =   14,
	Interrupt15_IRQn               =   15,
	Interrupt16_IRQn               =   16,
	Interrupt17_IRQn               =   17,
	Interrupt18_IRQn               =   18,
	Interrupt19_IRQn               =   19,

	/* 20-29 */
	Interrupt20_IRQn               =   20,
	Interrupt21_IRQn               =   21,
	Interrupt22_IRQn               =   22,
	Interrupt23_IRQn               =   23,
	Interrupt24_IRQn               =   24,
	Interrupt25_IRQn               =   25,
	Interrupt26_IRQn               =   26,
	Interrupt27_IRQn               =   27,
	Interrupt28_IRQn               =   28,
	Interrupt29_IRQn               =   29,

	/* 30-39 */
	Interrupt30_IRQn               =   30,
	Interrupt31_IRQn               =   31,
	Interrupt32_IRQn               =   32,
	Interrupt33_IRQn               =   33,
	Interrupt34_IRQn               =   34,
	Interrupt35_IRQn               =   35,
	Interrupt36_IRQn               =   36,
	Interrupt37_IRQn               =   37,
	Interrupt38_IRQn               =   38,
	Interrupt39_IRQn               =   39,

	/* 40-49 */
	Interrupt40_IRQn               =   40,
	Interrupt41_IRQn               =   41,
	Interrupt42_IRQn               =   42,
	Interrupt43_IRQn               =   43,
	Interrupt44_IRQn               =   44,
	Interrupt45_IRQn               =   45,
	Interrupt46_IRQn               =   46,
	Interrupt47_IRQn               =   47,
	Interrupt48_IRQn               =   48,
	Interrupt49_IRQn               =   49,

	/* 50-59 */
	Interrupt50_IRQn               =   50,
	Interrupt51_IRQn               =   51,
	Interrupt52_IRQn               =   52,
	Interrupt53_IRQn               =   53,
	Interrupt54_IRQn               =   54,
	Interrupt55_IRQn               =   55,
	Interrupt56_IRQn               =   56,
	Interrupt57_IRQn               =   57,
	Interrupt58_IRQn               =   58,
	Interrupt59_IRQn               =   59,

	/* 60-69 */
	Interrupt60_IRQn               =   60,
	Interrupt61_IRQn               =   61,
	Interrupt62_IRQn               =   62,
	Interrupt63_IRQn               =   63,
	Interrupt64_IRQn               =   64,
	Interrupt65_IRQn               =   65,
	Interrupt66_IRQn               =   66,
	Interrupt67_IRQn               =   67,
	Interrupt68_IRQn               =   68,
	Interrupt69_IRQn               =   69,

	/* 70-79 */
	Interrupt70_IRQn               =   70,
	Interrupt71_IRQn               =   71,
	Interrupt72_IRQn               =   72,
	Interrupt73_IRQn               =   73,
	Interrupt74_IRQn               =   74,
	Interrupt75_IRQn               =   75,
	Interrupt76_IRQn               =   76,
	Interrupt77_IRQn               =   77,
	Interrupt78_IRQn               =   78,
	Interrupt79_IRQn               =   79,

	/* 80-89 */
	Interrupt80_IRQn               =   80,
	Interrupt81_IRQn               =   81,
	Interrupt82_IRQn               =   82,
	Interrupt83_IRQn               =   83,
	Interrupt84_IRQn               =   84,
	Interrupt85_IRQn               =   85,
	Interrupt86_IRQn               =   86,
	Interrupt87_IRQn               =   87,
	Interrupt88_IRQn               =   88,
	Interrupt89_IRQn               =   89,

	/* 90-99 */
	Interrupt90_IRQn               =   90,
	Interrupt91_IRQn               =   91,
	Interrupt92_IRQn               =   92,
	Interrupt93_IRQn               =   93,
	Interrupt94_IRQn               =   94,
	Interrupt95_IRQn               =   95,
	Interrupt96_IRQn               =   96,
	Interrupt97_IRQn               =   97,
	Interrupt98_IRQn               =   98,
	Interrupt99_IRQn               =   99,

	/* 100-109 */
	Interrupt100_IRQn               =   100,
	Interrupt101_IRQn               =   101,
	Interrupt102_IRQn               =   102,
	Interrupt103_IRQn               =   103,
	Interrupt104_IRQn               =   104,
	Interrupt105_IRQn               =   105,
	Interrupt106_IRQn               =   106,
	Interrupt107_IRQn               =   107,
	Interrupt108_IRQn               =   108,
	Interrupt109_IRQn               =   109,

	/* 110-119 */
	Interrupt110_IRQn               =   110,
	Interrupt111_IRQn               =   111,
	Interrupt112_IRQn               =   112,
	Interrupt113_IRQn               =   113,
	Interrupt114_IRQn               =   114,
	Interrupt115_IRQn               =   115,
	Interrupt116_IRQn               =   116,
	Interrupt117_IRQn               =   117,
	Interrupt118_IRQn               =   118,
	Interrupt119_IRQn               =   119,

	/* 120-129 */
	Interrupt120_IRQn               =   120,
	Interrupt121_IRQn               =   121,
	Interrupt122_IRQn               =   122,
	Interrupt123_IRQn               =   123,
	Interrupt124_IRQn               =   124,
	Interrupt125_IRQn               =   125,
	Interrupt126_IRQn               =   126,
	Interrupt127_IRQn               =   127,
	Interrupt128_IRQn               =   128,
	Interrupt129_IRQn               =   129,

	/* 239 last interrupt */
	Interrupt239_IRQn		= 239,
	Interruptlast_IRQn		= 239,
} IRQn_Type;

/*****************************************************************/

#define __CM3_REV			0x0201U	/* Core revision r2p1 from CPUID:0x412FC231 */
#define __MPU_PRESENT			1U	/* MPU present */
#define __VTOR_PRESENT			1U	/* VTOR present and VTOR_OFFSET[31:7] round up 2-aligned */
#define __NVIC_PRIO_BITS		3U	/* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig		0U	/* Set to 1 if different SysTick Config is used */
/*****************************************************************/

enum {
	NVIC_PriorityGroup_0 = 0,	/* pre:Bit [7:1]   subpre: Bit[0]   */
	NVIC_PriorityGroup_1 = 1,	/* pre:Bit [7:2]   subpre: Bit[1:0] */
	NVIC_PriorityGroup_2 = 2,	/* pre:Bit [7:3]   subpre: Bit[2:0] */
	NVIC_PriorityGroup_3 = 3,	/* pre:Bit [7:4]   subpre: Bit[3:0] */
	NVIC_PriorityGroup_4 = 4,	/* pre:Bit [7:5]   subpre: Bit[4:0] */
	NVIC_PriorityGroup_5 = 5,	/* pre:Bit [7:6]   subpre: Bit[5:0] */
	NVIC_PriorityGroup_6 = 6,	/* pre:Bit [7]     subpre: Bit[6:0] */
	NVIC_PriorityGroup_7 = 7,	/* pre:Bit [NONE]  subpre: Bit[7:0] (all normal priority) */
};

/* __NVIC_PRIO_BITS = 3, 8-level.  Set NVIC_IPR0=0xFF => Get 0xE0. so 3 bit */
enum {
	NVIC_INTR_PRIORITY_00 = (0x00 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_20 = (0x20 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_40 = (0x40 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_60 = (0x60 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_80 = (0x80 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_A0 = (0xA0 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_C0 = (0xC0 >> (8 - __NVIC_PRIO_BITS)),
	NVIC_INTR_PRIORITY_E0 = (0xE0 >> (8 - __NVIC_PRIO_BITS)),
};
/*****************************************************************/
#include <cmsis/core_cm3.h>	/* Processor and core peripherals */
#include <cmsis/system.h>	/* System Header */

extern uint32_t g_cpufreq;
extern uint32_t g_apbfreq;
extern uint32_t g_ahbfreq;

#define CPUFREQ_REG	(28)	/* reuse the interrupt vector 28, reserved for CM3 */
#define AHBFREQ_REG	(32)	/* reuse the interrupt vector 32, reserved for CM3 */

static inline unsigned int current_cpufreq(void)
{
	if (g_cpufreq == (uint32_t)-1) {
		g_cpufreq = readl(CPUFREQ_REG);
	}
	return g_cpufreq;
}

static inline unsigned int current_apbfreq(void)
{
	/* apbfreq = ahbfreq >> 1 */
	if (g_apbfreq == (uint32_t)-1) {
		g_ahbfreq = readl(AHBFREQ_REG);
		g_apbfreq = g_ahbfreq >> 1;
	}

	return g_apbfreq;
}

static inline unsigned int current_ahbfreq(void)
{
	if (g_ahbfreq == (uint32_t)-1) {
		g_ahbfreq = readl(AHBFREQ_REG);
	}
	return g_ahbfreq;
}

static inline uint32_t current_cpuid(void)
{
	uint32_t cpuid = 0;

#if (CM3CORE_CLUSTERID != INVLIAD_ADDRESS)
	cpuid = readl(CM3CORE_CLUSTERID) & 3;
#endif
	return cpuid;
}

#endif

#endif
