#include "rtc.h"

void rtc_init() {
    // From https://wiki.osdev.org/RTC
    outb(0x70, 0x8B);		// select register B, and disable NMI
    char prev = inb(0x71);	// read the current value of register B
    outb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
    outb(0x71, prev | 0x40);// write the previous value ORed with 0x40. This turns on bit 6 of register B

    enable_irq(SLAVE_8259_PORT);
    enable_irq(RTC_IRQ);
}

void rtc_interrupt() {
    disable_irq(RTC_IRQ);
    disable_irq(SLAVE_8259_PORT);

    //test_interrupts();
    printf("tick ");

    outb(0x70, 0x0C);	// select register C
    inb(0x71);		// just throw away contents

    send_eoi(RTC_IRQ);
    send_eoi(SLAVE_8259_PORT);
    enable_irq(SLAVE_8259_PORT);
    enable_irq(RTC_IRQ);
}
