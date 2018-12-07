#include "qemu_vga.h"
#include "vga_text.h"
#include "../interrupts/multiprocessing.h"
#include "../data/vga_fonts.h"
#include "../data/color.h"

int qemu_vga_curr_bank = 0;
uint16_t qemu_vga_xres = 0;
uint16_t qemu_vga_yres = 0;
uint16_t qemu_vga_bpp = 0;
uint32_t qemu_vga_addr = 0;
uint32_t qemu_vga_cursor_x = 0;
uint32_t qemu_vga_cursor_y = 0;

uint16_t qemu_vga_read(uint16_t index) {
    outw(index, QEMU_VGA_PORT_INDEX);
    return inw(QEMU_VGA_PORT_DATA);
}

void qemu_vga_write(uint16_t index, uint16_t data) {
    outw(index, QEMU_VGA_PORT_INDEX);
    outw(data, QEMU_VGA_PORT_DATA);
}

uint32_t qemu_vga_active_window_addr() {
    return qemu_vga_addr + active_terminal_id * (qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp / BITS_IN_BYTE);
}

void qemu_vga_switch_terminal(int32_t tid) {
    qemu_vga_write(QEMU_VGA_IDX_Y_OFFSET, tid * qemu_vga_yres);
}

uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp) {
    if(0 == qemu_vga_addr) return FAIL;

    qemu_vga_write(QEMU_VGA_IDX_ID, QEMU_VGA_MAX_VER);
    uint16_t ver = qemu_vga_read(QEMU_VGA_IDX_ID);
    if(ver < QEMU_VGA_MIN_VER || ver > QEMU_VGA_MAX_VER) {
        return FAIL;
    }

    qemu_vga_xres = xres;
    qemu_vga_yres = yres;
    qemu_vga_bpp = bpp;

    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_DISABLE);
    qemu_vga_write(QEMU_VGA_IDX_XRES, xres);
    qemu_vga_write(QEMU_VGA_IDX_YRES, yres);
    qemu_vga_write(QEMU_VGA_IDX_BPP, bpp);
    qemu_vga_write(QEMU_VGA_IDX_VIRT_WIDTH, xres);
    qemu_vga_write(QEMU_VGA_IDX_X_OFFSET, 0);
    qemu_vga_write(QEMU_VGA_IDX_Y_OFFSET, 0);
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE_CLEAR);

    return SUCCESS;
}

void qemu_vga_pixel_set(uint16_t x, uint16_t y, vga_color_t color) {
    uint32_t pos = qemu_vga_active_window_addr() + (y * qemu_vga_xres + x) * qemu_vga_bpp / BITS_IN_BYTE;

    if(qemu_vga_bpp == 32) {
        // 32 bit encoding, 0x00RRGGBB
        *((uint32_t*) pos) = color.val & 0xffffff;
    } else if(qemu_vga_bpp == 16) {
        // 16 bit encoding, 5-6-5 as in MP2
        *((uint16_t*) pos) = color.val & 0xffff;
    }
}

void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg, vga_color_t bg) {
    // int screen_x = x * FONT_WIDTH;
    // int screen_y = y * FONT_HEIGHT;
    int i, j;
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

void qemu_vga_putc_transparent(uint16_t x, uint16_t y, uint8_t ch, vga_color_t fg) {
    // int screen_x = x * FONT_WIDTH;
    // int screen_y = y * FONT_HEIGHT;
    int i, j;
    for(i = 0; i < FONT_DATA_HEIGHT; i++) {
        for(j = 0; j < FONT_DATA_WIDTH; j++) {
            uint8_t mask = 1 << (7 - j);
            if(font_data[ch][i] & mask) {
                qemu_vga_pixel_set(x + j, y + i, fg);
            }
        }
    }
}

void qemu_vga_puts(uint16_t x, uint16_t y, uint8_t* s, uint16_t len, vga_color_t fg, vga_color_t bg) {
    int i;
    for(i = 0; i < len; i++) {
        qemu_vga_putc(x + FONT_ACTUAL_WIDTH * i, y, s[i], fg, bg);
    }
}

void qemu_vga_clear() {
    memset((char*) qemu_vga_active_window_addr(), 0,
        FONT_ACTUAL_HEIGHT * SCREEN_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE);
}

void qemu_vga_clear_row(uint8_t grid_y) {
    int pos_start = grid_y * FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE;
    memset((char*) (pos_start + qemu_vga_active_window_addr()), 0,
        FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE);
}

void qemu_vga_roll_up() {
    int pos_offset = FONT_ACTUAL_HEIGHT * qemu_vga_xres * qemu_vga_bpp / BITS_IN_BYTE;
    int len_roll = (SCREEN_HEIGHT - 1) * pos_offset;
    memcpy((char*) qemu_vga_active_window_addr(),
        (char*) (qemu_vga_active_window_addr() + pos_offset),
        len_roll);
    if(qemu_vga_cursor_y > 0) qemu_vga_cursor_y -= 1;
}

void qemu_vga_set_cursor_pos(uint8_t x, uint8_t y) {
    if(active_terminal_id != displayed_terminal_id) return;
    if(qemu_vga_cursor_x == x && qemu_vga_cursor_y == y) return;

    // erase cursor on current location by redrawing the character
    // uint8_t ch = *(uint8_t*) (TERMINAL_DIRECT_ADDR + (SCREEN_WIDTH * qemu_vga_cursor_y + qemu_vga_cursor_x) * 2);
    // uint8_t attrib = *(uint8_t*) (TERMINAL_DIRECT_ADDR + (SCREEN_WIDTH * qemu_vga_cursor_y + qemu_vga_cursor_x) * 2 + 1);
    // qemu_vga_putc(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     ch, qemu_vga_get_terminal_color(attrib), qemu_vga_get_terminal_color(attrib >> 4));

    // qemu_vga_putc_transparent(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     '_', qemu_vga_get_terminal_color(ATTRIB >> 4));

    qemu_vga_cursor_x = x;
    qemu_vga_cursor_y = y;

    // draw cursor on new location by drawing an underline
    // qemu_vga_putc_transparent(qemu_vga_cursor_x * FONT_ACTUAL_WIDTH,
    //     qemu_vga_cursor_y * FONT_ACTUAL_HEIGHT,
    //     '_', qemu_vga_get_terminal_color(ATTRIB));
}

vga_color_t qemu_vga_get_terminal_color(uint8_t color) {
    return (qemu_vga_bpp == 16 ? terminal_color_16 : terminal_color_32)[(color & 0xff)];
}
