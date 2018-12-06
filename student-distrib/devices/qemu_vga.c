#include "qemu_vga.h"
#include "vga_text.h"
#include "../data/vga_fonts.h"

int qemu_vga_curr_bank = 0;
int qemu_vga_enabled = 0;
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
    qemu_vga_write(QEMU_VGA_IDX_BANK, bank);
}

uint16_t qemu_vga_init(uint16_t xres, uint16_t yres, uint16_t bpp) {
    if(qemu_vga_enabled) return FAIL;

    uint16_t ver = qemu_vga_read(QEMU_VGA_IDX_ID);
    if(ver < QEMU_VGA_MIN_VER || ver > QEMU_VGA_MAX_VER) {
        return FAIL;
    }

    qemu_vga_xres = xres;
    qemu_vga_yres = yres;
    qemu_vga_bpp = bpp;

    qemu_vga_write(QEMU_VGA_IDX_XRES, xres);
    qemu_vga_write(QEMU_VGA_IDX_YRES, yres);
    qemu_vga_write(QEMU_VGA_IDX_BPP, bpp);
    qemu_vga_enabled = 0;

    return SUCCESS;
}

void qemu_vga_enable() {
    qemu_vga_enabled = 1;
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE);
}

void qemu_vga_disable() {
    qemu_vga_enabled = 0;
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_DISABLE);
}

void qemu_vga_enable_clear() {
    qemu_vga_enabled = 1;
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE_CLEAR);
}

void qemu_vga_pixel_set(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    if(!qemu_vga_enabled) return;
    uint32_t pos = (y * qemu_vga_xres + x) * (1.0 * qemu_vga_bpp / BITS_IN_BYTE);
    uint16_t bank = pos / QEMU_VGA_BANK_SIZE;
    pos = QEMU_VGA_MEM_POS + pos % QEMU_VGA_BANK_SIZE;
    qemu_vga_select_bank(bank);

    if(qemu_vga_bpp == 32) {
        // 32 bit encoding, 0x00RRGGBB
        uint8_t* ref = (uint8_t*) pos; *ref = b;
        ref++; *ref = g;
        ref++; *ref = r;
    } else if(qemu_vga_bpp == 16) {
        // 16 bit encoding, 5-6-5 as in MP2
        uint16_t* ref = (uint16_t*) pos;
        *ref = ((uint16_t) r & 0xF8) << 8
               | ((uint16_t) g & 0xFC) << 3
               | ((uint16_t) b & 0xF8) >> 3;
    }
}

void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch,
        uint8_t fr, uint8_t fg, uint8_t fb,
        uint8_t br, uint8_t bg, uint8_t bb) {
    if(!qemu_vga_enabled) return;
    // int screen_x = x * FONT_WIDTH;
    // int screen_y = y * FONT_HEIGHT;
    int i, j;
    for(i = 0; i < FONT_HEIGHT; i++) {
        for(j = 0; j < FONT_WIDTH; j++) {
            int grayscale = font_data[ch][i * FONT_WIDTH + j];
            uint16_t r_val = ((uint32_t) fr * grayscale + (uint32_t) br * (255 - grayscale)) / 255;
            uint16_t g_val = ((uint32_t) fg * grayscale + (uint32_t) bg * (255 - grayscale)) / 255;
            uint16_t b_val = ((uint32_t) fb * grayscale + (uint32_t) bb * (255 - grayscale)) / 255;
            qemu_vga_pixel_set(x + j, y + i, r_val, g_val, b_val);
        }
    }
}

void qemu_vga_puts(uint16_t x, uint16_t y, uint8_t* s, uint16_t len,
        uint8_t fr, uint8_t fg, uint8_t fb,
        uint8_t br, uint8_t bg, uint8_t bb) {
    if(!qemu_vga_enabled) return;
    int i;
    for(i = 0; i < len; i++) {
        qemu_vga_putc(x + FONT_WIDTH * i, y, s[i],
            fr, fg, fb, br, bg, bb);
    }
}
