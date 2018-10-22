#define ASM 1

#include "interrupt_wrap.h"

/* INTERRUPT_WRAP(name, func)
 * @input: name - name of interrupt wrapper function
 *         func - the function to be wrapped
 * @output: a function in name of *name*, ready to be inserted into IDT
 * @description: Wrap a function in PUSH/POPAL, PUSH/POPFL, IRET calls,
 *               so the interrupt can be handled normally
 */
#define INTERRUPT_WRAP(name, func) \
    .globl name                   ;\
    name:                         ;\
        pushal                    ;\
        pushfl                    ;\
        call func                 ;\
        popfl                     ;\
        popal                     ;\
        iret
        
INTERRUPT_WRAP(interrupt_rtc_wrap, rtc_interrupt);
INTERRUPT_WRAP(interrupt_keyboard_wrap, keyboard_interrupt);
