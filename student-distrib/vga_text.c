#include "vga_text.h"
#include "lib.h"

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

/*
 * enable_cursor:
 *   @input: cursor_start, cursor_end -  the rows where the cursor starts
 *           and ends. The highest scanline is 0 and the lowest scanline
 *           is the maximum scanline (usually 15).
 *   @output: none
 *   @description: function used to enable cursor
 *   @referemce: https://wiki.osdev.org/Text_Mode_Cursor
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
  cli();
	outb(0x0A, VGA_CURSOR_CONTROL_REG);
	outb((inb(VGA_CURSOR_DATA_REG) & 0xC0) | cursor_start, VGA_CURSOR_DATA_REG);

	outb(0x0B, VGA_CURSOR_CONTROL_REG);
	outb((inb(VGA_CURSOR_DATA_REG) & 0xE0) | cursor_end, VGA_CURSOR_DATA_REG);
  sti();
}

/*
 * enable_cursor:
 *   @input: none
 *   @output: none
 *   @description: function used to disable cursor
 *   @referemce: https://wiki.osdev.org/Text_Mode_Cursor
 */
void disable_cursor()
{
  cli();
	outb(0x0A, VGA_CURSOR_CONTROL_REG);
	outb(0x20, VGA_CURSOR_DATA_REG);
  sti();
}

/*
 * enable_cursor:
 *   @input: x,y - expected location of the cursor
 *   @output: none
 *   @description: function used to update cursor
 *   @notes: Keep in mind that you don't need to update the cursor's location
 *           every time a new character is displayed. It would be faster to
 *           instead only update it after printing an entire string.
 *   @referemce: https://wiki.osdev.org/Text_Mode_Cursor
 */
void update_cursor(int x, int y)
{
  cli();
	uint16_t pos = (y * SCREEN_WIDTH + x) % (SCREEN_WIDTH * SCREEN_HEIGHT);

	outb(0x0F, VGA_CURSOR_CONTROL_REG);
	outb((uint8_t) (pos & 0xFF), VGA_CURSOR_DATA_REG);
	outb(0x0E, VGA_CURSOR_CONTROL_REG);
	outb((uint8_t) ((pos >> 8) & 0xFF), VGA_CURSOR_DATA_REG);
  sti();
}
