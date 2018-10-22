#ifndef _VGA_TEXT_H_
#define _VGA_TEXT_H_

#include "types.h"

#define VIDEO         0xB8000
#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25

extern void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background);
extern void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch);

#endif
