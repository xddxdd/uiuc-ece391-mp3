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

#define QEMU_VGA_MEM_POS 0xa0000
#define QEMU_VGA_BANK_SIZE 0x10000

#define QEMU_VGA_MIN_VER 0xb0c0
#define QEMU_VGA_MAX_VER 0xb0c5

#define QEMU_VGA_DISABLE 0xe0
#define QEMU_VGA_ENABLE 0xe1
#define QEMU_VGA_ENABLE_CLEAR 0x61

#define BITS_IN_BYTE 8

extern int qemu_vga_enabled;
extern uint16_t qemu_vga_xres;
extern uint16_t qemu_vga_yres;
extern uint16_t qemu_vga_bpp;

uint16_t qemu_vga_read(uint16_t index);
void qemu_vga_write(uint16_t index, uint16_t data);
void qemu_vga_select_bank(uint16_t bank);

void qemu_vga_enable();
void qemu_vga_disable();
void qemu_vga_enable_clear();

uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp);
void qemu_vga_pixel_set(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch,
        uint8_t fr, uint8_t fg, uint8_t fb,
        uint8_t br, uint8_t bg, uint8_t bb);
void qemu_vga_puts(uint16_t x, uint16_t y, uint8_t* s, uint16_t len,
        uint8_t fr, uint8_t fg, uint8_t fb,
        uint8_t br, uint8_t bg, uint8_t bb);

#endif