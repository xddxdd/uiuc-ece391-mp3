#ifndef _RTL8139_H_
#define _RTL8139_H_

#include "../lib/lib.h"

#define RTL8139_REG_CONF1 (rtl_base_port + 0x52)
#define RTL8139_REG_RBSTART (rtl_base_port + 0x30)
#define RTL8139_REG_CMD (rtl_base_port + 0x37)
#define RTL8139_REG_IMR (rtl_base_port + 0x3c)
#define RTL8139_REG_ISR (rtl_base_port + 0x3e)
#define RTL8139_BUF_SIZE 10000

extern uint32_t rtl_base_port;
extern uint32_t rtl_interrupt_line;
extern uint8_t rtl_mac[6];
extern uint8_t rtl_buf[RTL8139_BUF_SIZE];

void rtl8139_init(uint32_t port, uint32_t int_line);

#endif
