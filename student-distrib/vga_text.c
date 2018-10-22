#include "vga_text.h"

/* void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background)
 * @input: x, y - coordinate of character whose color to be changed.
 *         foreground - foreground color to be changed, using VGA text mode palette.
 *         background - background color to be changed, using VGA text mode palette.
 * @output: the character at (x, y) has color changed to *foreground* and *background*
 * @description: change the color of character at (x, y) in VGA text mode.
 *               Initially used to draw Aqua on exception screen, but can be used for others.
 */
void vga_text_set_color(uint8_t x, uint8_t y, uint8_t foreground, uint8_t background) {
    uint8_t color = (background & 0xF) << 4 | (foreground & 0xF);
    *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2 + 1) = color;
}

/* void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch)
 * @input: x, y - coordinate of character whose color to be changed.
 *         ch - character to be set at (x, y).
 * @output: the character at (x, y) is changed to *ch*
 * @description: change the character at (x, y) in VGA text mode.
 *               Initially used to draw Aqua on exception screen, but can be used for others.
 */
void vga_text_set_character(uint8_t x, uint8_t y, uint8_t ch) {
    *(uint8_t*) (VIDEO + (y * SCREEN_WIDTH + x) * 2) = ch;
}
