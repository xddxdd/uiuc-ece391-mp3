#ifndef _QEMU_VGA_H_
#define _QEMU_VGA_H_

#include "../lib/lib.h"
#include "vga_text.h"

#define QEMU_VGA_PORT_INDEX 0x01ce
#define QEMU_VGA_PORT_DATA 0x01cf

#define QEMU_VGA_IDX_ID 0
#define QEMU_VGA_IDX_XRES 1
#define QEMU_VGA_IDX_YRES 2
#define QEMU_VGA_IDX_BPP 3
#define QEMU_VGA_IDX_ENABLE 4
#define QEMU_VGA_IDX_BANK 5
#define QEMU_VGA_IDX_VIRT_WIDTH 6
#define QEMU_VGA_IDX_VIRT_HEIGHT 7
#define QEMU_VGA_IDX_X_OFFSET 8
#define QEMU_VGA_IDX_Y_OFFSET 9

#define QEMU_VGA_BANK_SIZE 0x1000000

#define QEMU_VGA_MIN_VER 0xb0c0
#define QEMU_VGA_MAX_VER 0xb0c5

#define QEMU_VGA_DISABLE 0xe0
#define QEMU_VGA_ENABLE 0xe1
#define QEMU_VGA_ENABLE_CLEAR 0x61

#define BITS_IN_BYTE 8

#define FONT_ACTUAL_WIDTH 9
#define FONT_ACTUAL_HEIGHT 16

#define QEMU_VGA_DEFAULT_WIDTH 720
#define QEMU_VGA_DEFAULT_HEIGHT 400
#define QEMU_VGA_DEFAULT_BPP 32

extern int qemu_vga_enabled;
extern uint16_t qemu_vga_xres;
extern uint16_t qemu_vga_yres;
extern uint16_t qemu_vga_bpp;
extern uint32_t qemu_vga_addr;
extern uint32_t qemu_vga_cursor_x;
extern uint32_t qemu_vga_cursor_y;

typedef union {
    uint32_t val;
    struct __attribute__((packed)) {
        uint8_t r16         : 5;
        uint8_t g16         : 6;
        uint8_t b16         : 5;
        uint16_t dummy16    : 16;
    };
    struct __attribute__((packed)) {
        uint8_t r32;
        uint8_t g32;
        uint8_t b32;
        uint8_t dummy32;
    };
} vga_color_t;

uint16_t qemu_vga_read(uint16_t index);
void qemu_vga_write(uint16_t index, uint16_t data);

uint32_t qemu_vga_active_window_addr();
void qemu_vga_switch_terminal(int32_t tid);

uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp);
void qemu_vga_pixel_set(uint16_t x, uint16_t y, vga_color_t color);
void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg, vga_color_t bg);
void qemu_vga_putc_transparent(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg);
void qemu_vga_puts(uint16_t x, uint16_t y, uint8_t* s, uint16_t len, vga_color_t fg, vga_color_t bg);
void qemu_vga_clear();
void qemu_vga_clear_row(uint8_t grid_y);
void qemu_vga_roll_up();
void qemu_vga_set_cursor_pos(uint8_t x, uint8_t y);
vga_color_t qemu_vga_get_terminal_color(uint8_t color);

#endif
