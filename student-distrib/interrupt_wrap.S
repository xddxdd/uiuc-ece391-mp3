#define ASM 1

#include "interrupt_wrap.h"

#define INTERRUPT_WRAP(name, func) \
    .globl name                   ;\
    name:                         ;\
        pushal                    ;\
        pushfl                    ;\
        call func                 ;\
        popfl                     ;\
        popal                     ;\
        iret

INTERRUPT_WRAP(interrupt_rtc_wrap, interrupt_rtc);
INTERRUPT_WRAP(interrupt_keyboard_wrap, interrupt_keyboard);
