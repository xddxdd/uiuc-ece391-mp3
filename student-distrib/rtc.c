#include "rtc.h"

void rtc_init() {
    // From https://wiki.osdev.org/RTC
    outb(RTC_REG_B, RTC_PORT_CMD);		// select register B, and disable NMI
    char prev = inb(RTC_PORT_DATA);	// read the current value of register B
    outb(RTC_REG_B, RTC_PORT_CMD);		// set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_PORT_DATA);// write the previous value ORed with 0x40. This turns on bit 6 of register B

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
    outb(RTC_REG_A, RTC_PORT_CMD);
    old_freq = inb(RTC_PORT_DATA);
    outb(RTC_REG_A, RTC_PORT_CMD);
    outb((old_freq & 0xF0) | new_freq, RTC_PORT_DATA);
    sti();
}

void rtc_interrupt() {
    //test_interrupts();
    printf("tick ");

    outb(RTC_REG_C, RTC_PORT_CMD);	// select register C
    inb(RTC_PORT_DATA);		// just throw away contents

    send_eoi(RTC_IRQ);
}
