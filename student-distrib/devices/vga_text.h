#ifndef _VGA_TEXT_H_
#define _VGA_TEXT_H_

#include "../lib/types.h"

#define VIDEO         0xB8000
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

#define VGA_PORT_INDEX 0x3d4
#define VGA_PORT_DATA 0x3d5

#define VGA_REG_CURSOR_START 0x0a
#define VGA_REG_CURSOR_LOCATION_HIGH 0x0e
#define VGA_REG_CURSOR_LOCATION_LOW 0x0f

extern int screen_x;
extern int screen_y;

void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background);
void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch);
void vga_text_disable_cursor();
void vga_text_set_cursor_pos(uint8_t x, uint8_t y);

int32_t printf(int8_t *format, ...);
void putc(uint8_t c);
void roll_up();
void keyboard_echo(uint8_t c);
int32_t puts(int8_t *s);
void clear(void);
void test_interrupts();

#endif
