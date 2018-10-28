#include "vga_text.h"
#include "../lib.h"

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

/* void vga_text_disable_cursor()
 * @output: the cursor on VGA text mode is no longer there
 * @description: disable the blinking cursor.
 */
void vga_text_disable_cursor() {
    cli();
    // Read the value set at VGA's cursor start register,
    // by first sending an index and then reading the data port
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    char data = inb(VGA_PORT_DATA);
    // Replace the value of cursor start register with bit 5 set,
    // here bit 5 is the Cursor Disable field, and value of 1 means
    // the VGA text mode cursor is disabled
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    outb(0x20 | data, VGA_PORT_DATA);
    sti();
}

/* void vga_text_enable_cursor()
 * @output: the cursor on VGA text mode appears again
 * @description: enable the blinking cursor.
 */
void vga_text_enable_cursor() {
    cli();
    // Read the value set at VGA's cursor start register,
    // by first sending an index and then reading the data port
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    char data = inb(VGA_PORT_DATA);
    // Replace the value of cursor start register with bit 5 cleared,
    // here bit 5 is the Cursor Disable field, and value of 0 means
    // the VGA text mode cursor is enabled
    outb(VGA_REG_CURSOR_START, VGA_PORT_INDEX);
    outb(0xdf & data, VGA_PORT_DATA);
    sti();
}

/* void vga_text_set_cursor_pos(uint8_t x, uint8_t y)
 * @output: the cursor on VGA text mode changes to (x, y)
 * @description: set the position of the blinking cursor.
 */
void vga_text_set_cursor_pos(uint8_t x, uint8_t y) {
    cli();
    // Calculate the character position on VGA plane, row major order
    uint16_t char_pos = (y * SCREEN_WIDTH + x) % (SCREEN_WIDTH * SCREEN_HEIGHT);
    // Replace the value of VGA Cursor Location High Register with high 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_HIGH, VGA_PORT_INDEX);
    outb((uint8_t) (char_pos >> 8), VGA_PORT_DATA);
    // Replace the value of VGA Cursor Location Low Register with low 8 bits
    // of the calculated position
    outb(VGA_REG_CURSOR_LOCATION_LOW, VGA_PORT_INDEX);
    outb((uint8_t) char_pos, VGA_PORT_DATA);
    sti();
}
