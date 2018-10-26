#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "../lib.h"

#define COM1 0
#define COM2 1

#define SERIAL_OP_SUCCESS 0
#define SERIAL_OP_FAIL -1

int8_t serial_init(uint8_t id);
int8_t serial_is_available_rx(uint8_t id);
int8_t serial_is_available_tx(uint8_t id);
int8_t serial_read(uint8_t id);
int8_t serial_write(uint8_t id, uint8_t data);
void serial_interrupt(uint8_t id);
void serial1_interrupt();
void serial2_interrupt();

#endif
