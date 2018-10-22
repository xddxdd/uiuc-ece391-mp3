#include "vga_text.h"

void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background) {
    uint8_t color = (background & 0xF) << 4 | (foreground & 0xF);
    *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2 + 1) = color;
}

void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch) {
    *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2) = ch;
}
