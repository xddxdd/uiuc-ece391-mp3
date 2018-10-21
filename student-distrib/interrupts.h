#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "lib.h"

#ifndef ASM
    void interrupt_rtc();
    void interrupt_keyboard();
#endif

#endif
