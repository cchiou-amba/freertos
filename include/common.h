#ifndef __AMBA_COMMON_H__
#define __AMBA_COMMON_H__

/* interreupt trigger flag */
#define IRQ_RISING_EDGE		0
#define IRQ_FALLING_EDGE	1
#define IRQ_BOTH_EDGES		2
#define IRQ_LEVEL_LOW		3
#define IRQ_LEVEL_HIGH		4

#ifndef __ASM__

#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <FreeRTOS.h>

#define REF_CLK_FREQ		24000000 /* 24MHz, used by PLL */

#if defined(ARCH_CM3)
static inline int in_interrupt(void)
{
	return ((portNVIC_INT_CTRL_REG & 0xFF) != 0);
}
#elif defined(ARCH_R52)
static inline int in_interrupt(void)
{
    unsigned int mode;
    __asm volatile ("mrs %0, cpsr" : "=r"(mode));
    mode &= 0x1F;
    return (mode == 0x11 || mode == 0x12);
}
#else
#define in_interrupt()	(0)
#endif

extern int sprintf(char *out, const char *format, ...);
extern int printf(const char *format, ...);

#define pr_err(format...)						\
		do {							\
			printf("%s(%d): ", __func__, __LINE__);		\
			printf(format);					\
		} while(0)

#define ASSERT(x, format...)						\
		if ((x)) {						\
			printf("%s(%d): ", __func__, __LINE__);		\
			printf("Assertion: "format);			\
		}

#define BUG_ON(x, format...)						\
		if ((x)) {						\
			printf("%s(%d): ", __func__, __LINE__);		\
			printf("BUG_ON: "format);			\
			for (;;);					\
		}

#define DIE()	{ disable_interrupts(); for (;;);}


#define UINT_MAX		(~0U)
#define ULONG_MAX		(~0UL)

#define ATTR_SECTION(x) 	__attribute__((section(x)))
#define ATTR_UNUSED		__attribute__((unused))
#define ATTR_USED		__attribute__((used))
#define ATTR_WEAK		__attribute__((weak))
#define ATTR_ALIGN(n)		__attribute__((aligned(n)))

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#endif /* __ASM__ */

#endif
