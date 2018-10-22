#include "rtc.h"

/* void rtc_init()
 * @effects: Put RTC into working state
 * @description: Prepare the RTC for generating interrupts at specified interval
 */
void rtc_init() {
    // From https://wiki.osdev.org/RTC
    outb(RTC_REG_B, RTC_PORT_CMD);	    // select register B, and disable NMI
    char prev = inb(RTC_PORT_DATA);	    // read the current value of register B
    outb(RTC_REG_B, RTC_PORT_CMD);	    // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_PORT_DATA);   // write the previous value ORed with 0x40. This turns on bit 6 of register B

    enable_irq(RTC_IRQ);
}

/* uint8_t rtc_freq_to_config(uint16_t freq)
 * @input: freq - the desired frequency for RTC to send interrupts.
 *                can only be powers to 2, from 2 Hz to 1024 Hz.
 * @output: ret val - the data to be written into RTC register
 *                    to make it send interrupts at *freq* Hz.
 * @description: Generates the data to set RTC into desired frequency.
 *               The formula is (16 - log_2(freq)), where 2 <= freq <= 1024.
 *               Read more at https://wiki.osdev.org/RTC
 */
uint8_t rtc_freq_to_config(uint16_t freq) {
    switch(freq) {
        case 1024: return 0x06; // All calculated with retval = (16 - log_2(freq)),
        case 512:  return 0x07; // as stated above.
        case 256:  return 0x08; // As there's no ready to use log implementation,
        case 128:  return 0x09; // I'll just list out all possibilities.
        case 64:   return 0x0A;
        case 32:   return 0x0B;
        case 16:   return 0x0C;
        case 8:    return 0x0D;
        case 4:    return 0x0E;
        case 2:    return 0x0F;
        default:   return 0x0F; // If the input freq is invalid, return 2 Hz.
    }
}

/* void rtc_set_freq(uint16_t freq)
 * @input: freq - the desired frequency for RTC to send interrupts.
 *                can only be powers to 2, from 2 Hz to 1024 Hz.
 * @effects: RTC set to desired frequency.
 * @description: As stated above. Read more at https://wiki.osdev.org/RTC
 */
void rtc_set_freq(uint16_t freq) {
    uint8_t new_freq = rtc_freq_to_config(freq);    // Convert freq to RTC command
    uint8_t old_freq;
    cli();  // Enter critical section
    outb(RTC_REG_A, RTC_PORT_CMD);  // Read https://wiki.osdev.org/RTC for why
    old_freq = inb(RTC_PORT_DATA);  // we're doing this
    outb(RTC_REG_A, RTC_PORT_CMD);
    outb((old_freq & 0xF0) | new_freq, RTC_PORT_DATA);
    sti();  // Quit critical section
}

/* void rtc_interrupt()
 * @description: function to be called when RTC generates an interrupt.
 */
void rtc_interrupt() {
    //test_interrupts();

    // Read from RTC register C, so it can keep sending interrupts
    outb(RTC_REG_C, RTC_PORT_CMD); // select register C
    inb(RTC_PORT_DATA);		       // just throw away contents

    send_eoi(RTC_IRQ);  // And we're done
}
