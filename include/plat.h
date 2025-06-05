#ifndef __AMBA_PLAT_H__
#define __AMBA_PLAT_H__

#if defined(ARCH_CM3)
#include <cm3_plat.h>
#elif defined(ARCH_R52)
#include <r52_plat.h>
#else
#error "Undefined ARCH"
#endif

#endif
