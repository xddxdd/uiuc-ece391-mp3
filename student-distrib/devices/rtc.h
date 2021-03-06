#ifndef _RTC_H_
#define _RTC_H_

#include "../lib/lib.h"
#include "i8259.h"
#include "../fs/unified_fs.h"

// IRQ line connected to RTC, as used on x86 machines
#define RTC_IRQ 8

// RTC command & data port connected to these two ports, as used on x86 machines
#define RTC_PORT_CMD 0x70
#define RTC_PORT_DATA 0x71

#define RTC_FREQ_1024 0x06

#define RTC_FREQ_BASE 1024
#define RTC_FREQ_DEFAULT 2

// RTC register selector, as defined in specs
#define RTC_REG_A 0x8A
#define RTC_REG_B 0x8B
#define RTC_REG_C 0x8C

uint8_t rtc_init();
void rtc_interrupt();
void rtc_periodic_event();

// RTC Driver
int32_t rtc_open(int32_t* inode, char* filename);
int32_t rtc_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t rtc_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len);
int32_t rtc_close(int32_t* inode);

extern unified_fs_interface_t rtc_if;

#endif
