#include "qemu_vga.h"
#include "vga_text.h"
#include "../data/vga_fonts.h"

int qemu_vga_curr_bank = 0;
int qemu_vga_enabled = 0;

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

uint16_t qemu_vga_init() {
    if(qemu_vga_enabled) return QEMU_VGA_SUCCESS;

    uint16_t ver = qemu_vga_read(QEMU_VGA_IDX_ID);
    if(ver < QEMU_VGA_MIN_VER || ver > QEMU_VGA_MAX_VER) {
        return QEMU_VGA_FAIL;
    }

    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_DISABLE);
    qemu_vga_write(QEMU_VGA_IDX_XRES, QEMU_VGA_XRES);
    qemu_vga_write(QEMU_VGA_IDX_YRES, QEMU_VGA_YRES);
    qemu_vga_write(QEMU_VGA_IDX_BPP, QEMU_VGA_BPP);
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE_CLEAR);
    qemu_vga_enabled = 1;

    return QEMU_VGA_SUCCESS;
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
    uint32_t pos = (y * QEMU_VGA_XRES + x) * (QEMU_VGA_BPP / BITS_IN_BYTE);
    uint16_t bank = pos / QEMU_VGA_BANK_SIZE;
    pos = QEMU_VGA_MEM_POS + pos - bank * QEMU_VGA_BANK_SIZE;
    qemu_vga_select_bank(bank);

    if(QEMU_VGA_BPP == 32) {
        // 32 bit encoding, 0x00RRGGBB
        uint8_t* ref = (uint8_t*) pos; *ref = b;
        ref++; *ref = g;
        ref++; *ref = r;
    } else if(QEMU_VGA_BPP == 16) {
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
    int screen_x = x * FONT_WIDTH;
    int screen_y = y * FONT_HEIGHT;
    int i, j;
    for(i = 0; i < FONT_HEIGHT; i++) {
        for(j = 0; j < FONT_WIDTH; j++) {
            int grayscale = font_data[ch][i * FONT_WIDTH + j];
            uint16_t r_val = ((uint32_t) fr * grayscale + (uint32_t) br * (255 - grayscale)) / 255;
            uint16_t g_val = ((uint32_t) fg * grayscale + (uint32_t) bg * (255 - grayscale)) / 255;
            uint16_t b_val = ((uint32_t) fb * grayscale + (uint32_t) bb * (255 - grayscale)) / 255;
            qemu_vga_pixel_set(screen_x + j, screen_y + i, r_val, g_val, b_val);
        }
    }
}

char qemu_vga_scroll_tmp[QEMU_VGA_XRES * QEMU_VGA_BPP / BITS_IN_BYTE];

void qemu_vga_scroll_up(uint16_t y) {
    if(!qemu_vga_enabled) return;
    int i;
    uint32_t src_pos, tgt_pos, src_bank, tgt_bank;
    for(i = 0; i < FONT_HEIGHT * SCREEN_HEIGHT - y; i++) {
        src_pos = ((i + y) * QEMU_VGA_XRES) * (QEMU_VGA_BPP / BITS_IN_BYTE);
        src_bank = src_pos / QEMU_VGA_BANK_SIZE;
        src_pos = QEMU_VGA_MEM_POS + src_pos - src_bank * QEMU_VGA_BANK_SIZE;

        tgt_pos = (i * QEMU_VGA_XRES) * (QEMU_VGA_BPP / BITS_IN_BYTE);
        tgt_bank = tgt_pos / QEMU_VGA_BANK_SIZE;
        tgt_pos = QEMU_VGA_MEM_POS + tgt_pos - tgt_bank * QEMU_VGA_BANK_SIZE;

        if(src_bank == tgt_bank) {
            qemu_vga_select_bank(src_bank);
            memcpy((char*) tgt_pos, (char*) src_pos, QEMU_VGA_XRES * QEMU_VGA_BPP / BITS_IN_BYTE);
        } else {
            qemu_vga_select_bank(src_bank);
            memcpy(qemu_vga_scroll_tmp, (char*) src_pos, QEMU_VGA_XRES * QEMU_VGA_BPP / BITS_IN_BYTE);
            qemu_vga_select_bank(tgt_bank);
            memcpy((char*) tgt_pos, qemu_vga_scroll_tmp, QEMU_VGA_XRES * QEMU_VGA_BPP / BITS_IN_BYTE);
        }
    }
}
