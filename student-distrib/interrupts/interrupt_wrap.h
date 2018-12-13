#ifndef _INTERRUPT_WRAP_H_
#define _INTERRUPT_WRAP_H_

#ifndef ASM
    void interrupt_keyboard_wrap();
    void interrupt_rtc_wrap();
    void interrupt_serial1_wrap();
    void interrupt_serial2_wrap();
    void interrupt_sb16_wrap();
    void interrupt_pit_wrap();
    void interrupt_mouse_wrap();
#endif

#endif
