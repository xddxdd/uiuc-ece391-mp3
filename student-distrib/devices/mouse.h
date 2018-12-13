#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "../lib/lib.h"
#include "../fs/unified_fs.h"

#define MOUSE_REG_KEYBOARD      0x60
#define MOUSE_REG_PS2           0x64
#define MOUSE_GET_COMPAQ_STATUS 0x20
#define MOUSE_SET_COMPAQ_STATUS 0x60
#define MOUSE_CMD_SEND          0xd4
#define MOUSE_CMD_DEFAULT       0xf6
#define MOUSE_CMD_ENABLE_STREAM 0xf4
#define MOUSE_CMD_SAMPLE_RATE   0xf3
#define MOUSE_CMD_AUXILARY      0xa8
#define MOUSE_ACK               0xfa
#define MOUSE_IRQ               12

typedef union {
    int8_t val;
    struct __attribute__ ((packed)) {
        uint8_t btn_left    : 1;
        uint8_t btn_right   : 1;
        uint8_t btn_middle  : 1;
        uint8_t reserved    : 1;
        uint8_t x_sign      : 1;
        uint8_t y_sign      : 1;
        uint8_t x_overflow  : 1;
        uint8_t y_overflow  : 1;
    };
} mouse_t;

void mouse_reg_wait_out();
void mouse_reg_wait_in();
void mouse_reg_write(uint8_t data);
uint8_t mouse_reg_read();
void mouse_init();
void mouse_interrupt();

int32_t mouse_open(int32_t* inode, char* filename);
int32_t mouse_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t mouse_close(int32_t* inode);

extern unified_fs_interface_t mouse_if;

#endif
