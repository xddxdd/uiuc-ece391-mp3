#define ASM 1

#include "interrupt_wrap.h"

#define INTERRUPT_WRAP(name, func) \
    .globl name                   ;\
    name:                         ;\
        cli                       ;\
        pushal                    ;\
        pushfl                    ;\
        call func                 ;\
        popfl                     ;\
        popal                     ;\
        sti                       ;\
        iret

//INTERRUPT_WRAP(interrupt_rtc_wrap, rtc_interrupt);
INTERRUPT_WRAP(interrupt_keyboard_wrap, keyboard_interrupt);

.globl interrupt_rtc_wrap
interrupt_rtc_wrap:
    cli
    pushal
    pushfl
    call rtc_interrupt
    popfl
    popal
    sti
    iret
