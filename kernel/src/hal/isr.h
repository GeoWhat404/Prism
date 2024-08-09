#pragma once

#include <util/defines.h>

#if (ARCH == x86_64)
    #include <arch/x86/isr.h>
#else
    #error "Architecture not supported"
#endif
