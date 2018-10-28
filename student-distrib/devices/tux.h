#ifndef _TUX_H_
#define _TUX_H_

#define TUX_OP_SUCCESS 0
#define TUX_OP_FAIL -1

#define TC_SERIAL_PORT COM1
#define TC_SERIAL_BAUDRATE 9600

#include "../types.h"

extern uint8_t tc_buttons;

int8_t tux_init();
int8_t tux_set_led(char* word, uint8_t dot);
void tux_interrupt(char packet);

#endif
