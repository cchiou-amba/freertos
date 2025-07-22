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

#endif /* __ASM__ */

#endif
