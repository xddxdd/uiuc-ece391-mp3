#ifndef _VGA_TEXT_H_
#define _VGA_TEXT_H_

#include "types.h"

#define VIDEO         0xB8000
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

#define VGA_CURSOR_CONTROL_REG  0x3D4
#define VGA_CURSOR_DATA_REG     0x3D5

extern void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background);
extern void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch);

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);

#endif
