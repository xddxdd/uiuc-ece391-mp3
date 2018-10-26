#ifndef _INTERRUPT_WRAP_H_
#define _INTERRUPT_WRAP_H_

#ifndef ASM
    void interrupt_keyboard_wrap();
    void interrupt_rtc_wrap();
    void interrupt_serial1_wrap();
    void interrupt_serial2_wrap();
#endif

#endif
