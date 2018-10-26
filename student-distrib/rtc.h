#ifndef _RTC_H_
#define _RTC_H_

#include "lib.h"
#include "i8259.h"

// IRQ line connected to RTC, as used on x86 machines
#define RTC_IRQ 8

// RTC command & data port connected to these two ports, as used on x86 machines
#define RTC_PORT_CMD 0x70
#define RTC_PORT_DATA 0x71

// RTC register selector, as defined in specs
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C

void rtc_init();
uint8_t rtc_freq_to_config(uint16_t freq);
void rtc_set_freq(uint16_t freq);
void rtc_interrupt();

// RTC Driver
void rtc_open();
void rtc_read();
void rtc_write(uint16_t freq);
void rtc_close();

#endif
