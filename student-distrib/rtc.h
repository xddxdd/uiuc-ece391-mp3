#ifndef _RTC_H_
#define _RTC_H_

#include "lib.h"
#include "i8259.h"

#define RTC_IRQ 8

void rtc_init();
void rtc_interrupt();

#endif
