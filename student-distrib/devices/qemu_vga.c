#include "qemu_vga.h"
#include "vga_text.h"
#include "../data/vga_fonts.h"

int qemu_vga_curr_bank = 0;
uint16_t qemu_vga_xres = 0;
uint16_t qemu_vga_yres = 0;
uint16_t qemu_vga_bpp = 0;

uint16_t qemu_vga_read(uint16_t index) {
    outw(index, QEMU_VGA_PORT_INDEX);
    return inw(QEMU_VGA_PORT_DATA);
}

void qemu_vga_write(uint16_t index, uint16_t data) {
    outw(index, QEMU_VGA_PORT_INDEX);
    outw(data, QEMU_VGA_PORT_DATA);
}

void qemu_vga_select_bank(uint16_t bank) {
    if(bank == qemu_vga_curr_bank) return;
    qemu_vga_curr_bank = bank;
    // qemu_vga_write(QEMU_VGA_IDX_BANK, bank);
}

uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp) {
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
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE_CLEAR);

    return SUCCESS;
}

void qemu_vga_pixel_set(uint16_t x, uint16_t y, vga_color_t color) {
    uint32_t pos = (y * qemu_vga_xres + x) * (1.0 * qemu_vga_bpp / BITS_IN_BYTE);
    uint16_t bank = pos / QEMU_VGA_BANK_SIZE;
    pos = QEMU_VGA_MEM_POS + pos % QEMU_VGA_BANK_SIZE;
    qemu_vga_select_bank(bank);

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

void qemu_vga_puts(uint16_t x, uint16_t y, uint8_t* s, uint16_t len, vga_color_t fg, vga_color_t bg) {
    int i;
    for(i = 0; i < len; i++) {
        qemu_vga_putc(x + FONT_ACTUAL_WIDTH * i, y, s[i], fg, bg);
    }
}

void qemu_vga_clear() {
    int bank;
    for(bank = 0; bank * QEMU_VGA_BANK_SIZE < qemu_vga_xres * qemu_vga_yres * qemu_vga_bpp / BITS_IN_BYTE; bank++) {
        qemu_vga_select_bank(bank);
        memset(QEMU_VGA_MEM_POS, 0, QEMU_VGA_BANK_SIZE);
    }
}

void qemu_vga_clear_row(uint8_t grid_y) {
    int pos_start = grid_y * FONT_ACTUAL_HEIGHT * qemu_vga_xres;
    int pos_end = (grid_y + 1) * FONT_ACTUAL_HEIGHT * qemu_vga_xres;
    int bank_start = pos_start / QEMU_VGA_BANK_SIZE;
    int bank_end = pos_end / QEMU_VGA_BANK_SIZE;
    int bank;
    for(bank = bank_start; bank < bank_end; bank++) {
        int this_start = bank * QEMU_VGA_BANK_SIZE;
        int this_end = (bank + 1) * QEMU_VGA_BANK_SIZE;
        if(pos_start > this_start) this_start = pos_start;
        if(pos_end < this_end) this_end = pos_end;
        this_start -= bank * QEMU_VGA_BANK_SIZE;
        this_end -= bank * QEMU_VGA_BANK_SIZE;
        memset(QEMU_VGA_MEM_POS + this_start, 0, this_end - this_start);
    }
}
