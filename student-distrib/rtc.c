#include "rtc.h"

void rtc_init() {
    // From https://wiki.osdev.org/RTC
    outb(0x70, 0x8B);		// select register B, and disable NMI
    char prev = inb(0x71);	// read the current value of register B
    outb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
    outb(0x71, prev | 0x40);// write the previous value ORed with 0x40. This turns on bit 6 of register B

    rtc_set_freq(2);

    enable_irq(RTC_IRQ);
}

uint8_t rtc_freq_to_config(uint16_t freq) {
    switch(freq) {
        case 1024: return 0x06;
        case 512:  return 0x07;
        case 256:  return 0x08;
        case 128:  return 0x09;
        case 64:   return 0x0A;
        case 32:   return 0x0B;
        case 16:   return 0x0C;
        case 8:    return 0x0D;
        case 4:    return 0x0E;
        case 2:    return 0x0F;
        default:   return 0x0F;
    }
}

void rtc_set_freq(uint16_t freq) {
    uint8_t new_freq = rtc_freq_to_config(freq);
    uint8_t old_freq;
    cli();
    outb(0x70, 0x8A);
    old_freq = inb(0x71);
    outb(0x70, 0x8A);
    outb(0x71, (old_freq & 0xF0) | new_freq);
    sti();
}

void rtc_interrupt() {
    disable_irq(RTC_IRQ);

    //test_interrupts();
    printf("tick ");

    outb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents

    send_eoi(RTC_IRQ);
    enable_irq(RTC_IRQ);
}
