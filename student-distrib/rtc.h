#ifndef _RTC_H_
#define _RTC_H_

#include "lib.h"
#include "i8259.h"

#define RTC_IRQ 8

void rtc_init();
uint8_t rtc_freq_to_config(uint16_t freq);
void rtc_set_freq(uint16_t freq);
void rtc_interrupt();

#endif
