#include "qemu_vga.h"
#include "vga_text.h"
#include "../interrupts/multiprocessing.h"
#include "../data/vga_fonts.h"
#include "../data/color.h"
#include "../data/chinese_font.h"

// Address of linear buffer, set by PCI scanner on startup
uint32_t qemu_vga_addr = 0;

// Current status of QEMU VGA
uint16_t qemu_vga_xres = 0;
uint16_t qemu_vga_yres = 0;
uint16_t qemu_vga_bpp = 0;
uint32_t qemu_vga_enabled = 0;
uint32_t qemu_vga_cursor_x = 0;
uint32_t qemu_vga_cursor_y = 0;

/* uint16_t qemu_vga_read(uint16_t index)
 * @input: index - index of register in QEMU VGA
 * @output: ret val - data in that register
 * @description: read from a QEMU VGA register.
 */
uint16_t qemu_vga_read(uint16_t index) {
    outw(index, QEMU_VGA_PORT_INDEX);
    return inw(QEMU_VGA_PORT_DATA);
}


/* void qemu_vga_write(uint16_t index, uint16_t data)
 * @input: index - index of register in QEMU VGA
 *         data - data to be written into
 * @output: data written into the register
 * @description: write to a QEMU VGA register.
 */
void qemu_vga_write(uint16_t index, uint16_t data) {
    outw(index, QEMU_VGA_PORT_INDEX);
    outw(data, QEMU_VGA_PORT_DATA);
}

/* uint32_t qemu_vga_active_window_addr()
 * @output: ret val - address of the active window on QEMU VGA linear buffer.
 * @description: calculates and returns said address.
 */
uint32_t qemu_vga_active_window_addr() {
    return qemu_vga_addr + active_terminal_id * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp / BITS_IN_BYTE);
}

/* void qemu_vga_switch_terminal(int32_t tid)
 * @input: tid - terminal id
 * @output: display switches to the specified terminal
 * @description: Terminals are stored continuously in linear buffer:
 * +------------+
 * | Terminal 1 |
 * +------------+
 * | Terminal 2 |
 * +------------+
 * | Terminal 3 |
 * +------------+
 * so with a change of Y display offset, we can switch between these terminals.
 */
void qemu_vga_switch_terminal(int32_t tid) {
    if(tid >= TERMINAL_COUNT) return;
    qemu_vga_write(QEMU_VGA_IDX_Y_OFFSET, tid * qemu_vga_yres);
}

/* uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp)
 * @input: xres - X resolution
 *         yres - Y resolution
 *         bpp - bits per pixel, can only be 4, 8, 15, 16, 24, 32,
 *               where this OS only supports 16 and 32.
 * @output: ret val - SUCCESS / FAIL
 *          QEMU VGA initialized to said state
 * @description: initialized QEMU VGA.
 */
uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp) {
    if(0 == qemu_vga_addr) return FAIL;

    // Check if QEMU VGA device is present, and version correct
    qemu_vga_write(QEMU_VGA_IDX_ID, QEMU_VGA_MAX_VER);
    uint16_t ver = qemu_vga_read(QEMU_VGA_IDX_ID);
    if(ver < QEMU_VGA_MIN_VER || ver > QEMU_VGA_MAX_VER) {
        return FAIL;
    }

    // Currently only supports 16 bit and 32 bit BPP.
    // No plan to add more support as we don't use them.
    if(bpp != 16 && bpp != 32) return FAIL;

    // Store state information for later address calculation
    qemu_vga_xres = xres;
    qemu_vga_yres = yres;
    qemu_vga_bpp = bpp;

    // Write the setting into VGA
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_DISABLE);
    qemu_vga_write(QEMU_VGA_IDX_XRES, xres);
    qemu_vga_write(QEMU_VGA_IDX_YRES, yres);
    qemu_vga_write(QEMU_VGA_IDX_BPP, bpp);
    qemu_vga_write(QEMU_VGA_IDX_VIRT_WIDTH, xres);
    qemu_vga_write(QEMU_VGA_IDX_X_OFFSET, 0);
    qemu_vga_write(QEMU_VGA_IDX_Y_OFFSET, 0);
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE_CLEAR);
    qemu_vga_enabled = 1;

    return SUCCESS;
}

/* void qemu_vga_pixel_set(uint16_t x, uint16_t y, vga_color_t color)
 * @input: x, y - coordinate
 *         color: to be set for this pixel
 * @output: pixel (x, y) set to color specified
 * @description: sets a pixel.
 */
void qemu_vga_pixel_set(uint16_t x, uint16_t y, vga_color_t color) {
    if(x >= qemu_vga_xres || y >= qemu_vga_yres) return;
    uint32_t pos = qemu_vga_active_window_addr() + (y * qemu_vga_xres + x) * qemu_vga_bpp / BITS_IN_BYTE;

    // Currently only 32 bit and 16 bit color depth is supported.
    if(qemu_vga_bpp == 32) {
        // 32 bit encoding, 0x00RRGGBB
        *((uint32_t*) pos) = color.val & 0xffffff;
    } else if(qemu_vga_bpp == 16) {
        // 16 bit encoding, 5-6-5 as in MP2
        *((uint16_t*) pos) = color.val & 0xffff;
    }
}

/* void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg, vga_color_t bg)
 * @input: x, y - left top corner coordinate for the character
 *         ch - character to be displayed
 *         fg, bg - foreground and background color
 * @output: character written at specified position
 * @description: writes a character onto the screen.
 *     Supports handling UTF-8 encoded Chinese characters and missing ones.
 *     NOTE: Shares UTF-8 state with transparent putc function.
 */
void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg, vga_color_t bg) {
    int i, j;
    volatile utf8_state_t* utf8_state = &terminals[active_terminal_id].utf8_state;
    if(UTF8_3BYTE_MASK == (ch & UTF8_3BYTE_MASK)) {
        // Detected the beginning of a 3 byte UTF-8 code
        utf8_state->len = 3;
        utf8_state->have = 1;
        utf8_state->buf[0] = ch;
        return;
    } else if(UTF8_2BYTE_MASK == (ch & UTF8_2BYTE_MASK)) {
        // Detected the beginning of a 2 byte UTF-8 code
        utf8_state->len = 2;
        utf8_state->have = 1;
        utf8_state->buf[0] = ch;
        return;
    } else if(utf8_state->len > 0) {
        // Part of an incomplete UTF-8 code
        utf8_state->buf[utf8_state->have++] = ch;
        if(utf8_state->have == utf8_state->len) {
            // UTF-8 code is complete, decode and print it
            // according to UTF-8 coding scheme
            uint16_t code = 0;
            if(3 == utf8_state->len) {
                code = (utf8_state->buf[0] & 0xf) << 12
                     | (utf8_state->buf[1] & 0x3f) << 6
                     | (utf8_state->buf[2] & 0x3f);
            } else if(2 == utf8_state->len) {
                code = (utf8_state->buf[0] & 0x1f) << 6
                     | (utf8_state->buf[1] & 0x3f);
            }

            // Clear state: we're not waiting for another code
            utf8_state->len = 0;
            utf8_state->have = 0;

            if(code >= CHINESE_ENCODE_START && code < CHINESE_ENCODE_END) {
                // This is a Chinese character, load font and print it
                uint16_t* font = (uint16_t*) (CHINESE_FONT_DATA + (code - CHINESE_ENCODE_START)
                    * (CHINESE_FONT_HEIGHT * CHINESE_FONT_WIDTH / 8));

                // Draw the character
                for(i = 0; i < CHINESE_FONT_HEIGHT; i++) {
                    uint16_t font_line = (font[i] >> 8) | (font[i] << 8);
                    for(j = 0; j < CHINESE_FONT_WIDTH; j++) {
                        uint16_t mask = 1 << (15 - j);
                        if(font_line & mask) {
                            qemu_vga_pixel_set(x + CHINESE_FONT_LEFT + j, y + i, fg);
                        } else {
                            qemu_vga_pixel_set(x + CHINESE_FONT_LEFT + j, y + i, bg);
                        }
                    }
                }
            }
        }
    } else {
        // ASCII character, simply print it out
        for(i = 0; i < FONT_DATA_HEIGHT; i++) {
            for(j = 0; j < FONT_DATA_WIDTH; j++) {
                uint8_t mask = 1 << (7 - j);
                if(font_data[ch][i] & mask) {
                    qemu_vga_pixel_set(x + j, y + i, fg);
                } else {
                    qemu_vga_pixel_set(x + j, y + i, bg);
                }
            }
        }
    }
}

/* void qemu_vga_putc_transparent(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg)
 * @input: x, y - left top corner coordinate for the character
 *         ch - character to be displayed
 *         fg - foreground color
 * @output: character written at specified position
 * @description: writes a character onto the screen, without background color.
 *     Supports handling UTF-8 encoded Chinese characters and missing ones.
 *     NOTE: Shares UTF-8 state with non transparent putc function.
 */
void qemu_vga_putc_transparent(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg) {
    int i, j;
    volatile utf8_state_t* utf8_state = &terminals[active_terminal_id].utf8_state;
    if(UTF8_3BYTE_MASK == (ch & UTF8_3BYTE_MASK)) {
        // Detected the beginning of a 3 byte UTF-8 code
        utf8_state->len = 3;
        utf8_state->have = 1;
        utf8_state->buf[0] = ch;
        return;
    } else if(UTF8_2BYTE_MASK == (ch & UTF8_2BYTE_MASK)) {
        // Detected the beginning of a 2 byte UTF-8 code
        utf8_state->len = 2;
        utf8_state->have = 1;
        utf8_state->buf[0] = ch;
        return;
    } else if(utf8_state->len > 0) {
        // Part of an incomplete UTF-8 code
        utf8_state->buf[utf8_state->have++] = ch;
        if(utf8_state->have == utf8_state->len) {
            // UTF-8 code is complete, decode and print it
            // according to UTF-8 coding scheme
            uint16_t code = 0;
            if(3 == utf8_state->len) {
                code = (utf8_state->buf[0] & 0xf) << 12
                     | (utf8_state->buf[1] & 0x3f) << 6
                     | (utf8_state->buf[2] & 0x3f);
            } else if(2 == utf8_state->len) {
                code = (utf8_state->buf[0] & 0x1f) << 6
                     | (utf8_state->buf[1] & 0x3f);
            }

            // Clear state: we're not waiting for another code
            utf8_state->len = 0;
            utf8_state->have = 0;

            if(code >= CHINESE_ENCODE_START && code < CHINESE_ENCODE_END) {
                // This is a Chinese character, load font and print it
                uint16_t* font = (uint16_t*) (CHINESE_FONT_DATA + (code - CHINESE_ENCODE_START)
                    * (CHINESE_FONT_HEIGHT * CHINESE_FONT_WIDTH / 8));

                // Draw the character
                for(i = 0; i < CHINESE_FONT_HEIGHT; i++) {
                    uint16_t font_line = (font[i] >> 8) | (font[i] << 8);
                    for(j = 0; j < CHINESE_FONT_WIDTH; j++) {
                        uint16_t mask = 1 << (15 - j);
                        if(font_line & mask) {
                            qemu_vga_pixel_set(x + CHINESE_FONT_LEFT + j, y + i, fg);
                        }
                    }
                }
            }
        }
    } else {
        // ASCII character, simply print it out
        for(i = 0; i < FONT_DATA_HEIGHT; i++) {
            for(j = 0; j < FONT_DATA_WIDTH; j++) {
                uint8_t mask = 1 << (7 - j);
                if(font_data[ch][i] & mask) {
                    qemu_vga_pixel_set(x + j, y + i, fg);
                }
            }
        }
    }
}

/* void qemu_vga_clear()
 * @output: screen filled with black
 * @description: clear the current virtual screen.
 */
void qemu_vga_clear() {
    memset((char*) qemu_vga_active_window_addr(), 0,
        FONT_ACTUAL_HEIGHT * SCREEN_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE);
}

/* void qemu_vga_clear_row(uint8_t grid_y)
 * @input: grid_y - Y on text mode grid, range 0-24.
 * @output: that bar/row filled with black
 * @description: clear the row on screen.
 */
void qemu_vga_clear_row(uint8_t grid_y) {
    int pos_start = grid_y * FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE;
    memset((char*) (pos_start + qemu_vga_active_window_addr()), 0,
        FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE);
}

/* void qemu_vga_roll_up()
 * @output: screen rolls up one row.
 * @description: as above. Note that if there's extra space below the text area,
 *     they will not be touched. Useful for status bars.
 */
void qemu_vga_roll_up() {
    int pos_offset = FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE;
    int len_roll = (SCREEN_HEIGHT - 1) * pos_offset;
    memcpy((char*) qemu_vga_active_window_addr(),
        (char*) (qemu_vga_active_window_addr() + pos_offset),
        len_roll);
    if(qemu_vga_cursor_y > 0) qemu_vga_cursor_y -= 1;
}

/* void qemu_vga_set_cursor_pos(uint8_t x, uint8_t y)
 * @input: x, y - cursor coordinate
 * @output: cursor moved to that position (currently NOT WORKING)
 * @description: simulates VGA cursor, NOT WORKING NOW
 */
void qemu_vga_set_cursor_pos(uint8_t x, uint8_t y) {
    if(active_terminal_id != displayed_terminal_id) return;
    if(qemu_vga_cursor_x == x && qemu_vga_cursor_y == y) return;

    // // erase cursor on current location by redrawing the character
    // uint8_t ch = *(volatile uint8_t*) (TERMINAL_DIRECT_ADDR + (SCREEN_WIDTH * qemu_vga_cursor_y + qemu_vga_cursor_x) * 2);
    // uint8_t attrib = *(volatile uint8_t*) (TERMINAL_DIRECT_ADDR + (SCREEN_WIDTH * qemu_vga_cursor_y + qemu_vga_cursor_x) * 2 + 1);
    // qemu_vga_putc(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     ch, qemu_vga_get_terminal_color(attrib), qemu_vga_get_terminal_color(attrib >> 4));

    // qemu_vga_putc_transparent(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     '_', qemu_vga_get_terminal_color(ATTRIB >> 4));

    qemu_vga_cursor_x = x;
    qemu_vga_cursor_y = y;

    // // draw cursor on new location by drawing an underline
    // qemu_vga_putc_transparent(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     '_', qemu_vga_get_terminal_color(ATTRIB));
}

/* vga_color_t qemu_vga_get_terminal_color(uint8_t color)
 * @input: color - color code for console, only last 8 bit used
 * @output: ret val - color in 16 bit or 32 bit, depending on init setting
 * @description: translates terminal color to console color
 */
vga_color_t qemu_vga_get_terminal_color(uint8_t color) {
    return (qemu_vga_bpp == 16 ? terminal_color_16 : terminal_color_32)[(color & 0xff)];
}

/* void qemu_vga_show_picture(uint16_t width, uint16_t height, uint8_t bpp, uint8_t* data)
 * @input: width, height - size of picture, cannot exceed screen resolution.
 *         bpp - color depth, MUST BE same as screen, or strange problem will occur
 *         data - data of image
 * @output: picture drawn on left top corner of screen,
 *          cursor moved downwards if overlapping with picture
 * @description: show a picture at left top corner of the screen.
 */
void qemu_vga_show_picture(uint16_t width, uint16_t height, uint8_t bpp, uint8_t* data) {
    if(width > qemu_vga_xres || height > qemu_vga_yres || bpp != qemu_vga_bpp) return;

    // Copy over the image, row by row
    int row = 0;
    int i;
    for(i = 0; i < height; i++) {
        memcpy((char*) (qemu_vga_active_window_addr() + row),
            (char*) (data + i * width * bpp / BITS_IN_BYTE), width * bpp / BITS_IN_BYTE);
        row += qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE;
    }

    // Move cursor downwards to avoid overlapping with picture
    if(terminals[active_terminal_id].screen_y < height / FONT_ACTUAL_HEIGHT) {
        terminals[active_terminal_id].screen_y = height / FONT_ACTUAL_HEIGHT + 1;
        terminals[active_terminal_id].screen_x = 0;
    }
}
