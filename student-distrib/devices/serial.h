#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "../lib/lib.h"

#define COM1 0
#define COM2 1

#define SERIAL_OP_SUCCESS 0
#define SERIAL_OP_FAIL -1

#define SERIAL_REG_OFFSET_DATA 0
#define SERIAL_REG_OFFSET_INTERRUPT_ENABLE 1
#define SERIAL_REG_OFFSET_BAUDRATE_LOWBYTE 0
#define SERIAL_REG_OFFSET_BAUDRATE_HIGHBYTE 1
#define SERIAL_REG_OFFSET_INTERRUPT_IDENTIFICATION 2
#define SERIAL_REG_OFFSET_LINE_CONTROL 3
#define SERIAL_REG_OFFSET_MODEM_CONTROL 4
#define SERIAL_REG_OFFSET_LINE_STATUS 5
#define SERIAL_REG_OFFSET_MODEM_STATUS 6
#define SERIAL_REG_OFFSET_SCRATCH 7

#define SERIAL_BASE_BAUDRATE 115200

#define SERIAL_LINE_CONTROL_DLAB 0x80
#define SERIAL_LINE_CONTROL_8N1 0x03

#define SERIAL_INTERRUPT_DISABLE 0x00
#define SERIAL_INTERRUPT_ENABLE_DATA 0x01

#define SERIAL_INTERRUPT_ID 0xc7
#define SERIAL_MODEM_CONTROL 0x0b

int8_t serial_init(uint8_t id, uint32_t baud_rate);
int8_t serial_is_available_rx(uint8_t id);
int8_t serial_is_available_tx(uint8_t id);
int8_t serial_read(uint8_t id);
int8_t serial_write(uint8_t id, uint8_t data);
void serial_interrupt(uint8_t id);
void serial1_interrupt();
void serial2_interrupt();

#endif
