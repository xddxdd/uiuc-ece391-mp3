#include "qemu_vga.h"
#include "../data/vga_fonts.h"

int qemu_vga_curr_bank = 0;

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
    uint16_t ver = qemu_vga_read(QEMU_VGA_IDX_ID);
    if(ver < QEMU_VGA_MIN_VER || ver > QEMU_VGA_MAX_VER) {
        return QEMU_VGA_FAIL;
    }

    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_DISABLE);
    qemu_vga_write(QEMU_VGA_IDX_XRES, QEMU_VGA_XRES);
    qemu_vga_write(QEMU_VGA_IDX_YRES, QEMU_VGA_YRES);
    qemu_vga_write(QEMU_VGA_IDX_BPP, QEMU_VGA_BPP);
    qemu_vga_write(QEMU_VGA_IDX_ENABLE, QEMU_VGA_ENABLE);

    uint8_t i;
    for(i = 0; i < 255; i++) {
        qemu_vga_putc(i % 16, i / 16, (uint8_t) i,
            255, 255, 255, 0, 0, 0);
    }

    return QEMU_VGA_SUCCESS;
}

void qemu_vga_pixel_set(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t pos = (y * QEMU_VGA_XRES + x) * 4;
    uint16_t bank = pos / QEMU_VGA_BANK_SIZE;
    pos = QEMU_VGA_MEM_POS + pos - bank * QEMU_VGA_BANK_SIZE;
    qemu_vga_select_bank(bank);

    uint8_t* ref = (uint8_t*) pos; *ref = b;
    ref++; *ref = g;
    ref++; *ref = r;
}

void qemu_vga_putc(uint16_t x, uint16_t y, uint8_t ch,
        uint8_t fr, uint8_t fg, uint8_t fb,
        uint8_t br, uint8_t bg, uint8_t bb) {
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
