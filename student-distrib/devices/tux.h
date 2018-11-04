#ifndef _TUX_H_
#define _TUX_H_

#define TUX_OP_SUCCESS 0
#define TUX_OP_FAIL -1

#define TC_SERIAL_PORT COM1
#define TC_SERIAL_BAUDRATE 9600

#include "../types.h"
#include "../fs/unified_fs.h"

extern volatile uint8_t tc_buttons;

int8_t tux_init();
int8_t tux_set_led(char* word, uint8_t dot);
void tux_interrupt(char packet);

int32_t tux_open(int32_t* inode, char* filename);
int32_t tux_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t tux_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len);
int32_t tux_close(int32_t* inode);

extern unified_fs_interface_t tux_if;

#endif
