#include "chinese_input.h"
#include "../devices/qemu_vga.h"
#include "../devices/keyboard.h"
#include "../interrupts/multiprocessing.h"

void chinese_input_keystroke(uint8_t ch) {
    volatile chinese_input_buf_t* b = &terminals[displayed_terminal_id].chinese_input_buf;
    if(ch == BACKSPACE) {
        if(b->buf_len > 0) {
            b->buf_len--;
            b->buf[b->buf_len] = '\0';
            chinese_input_search();
        } else return;
    } else if(ch == ',') {
        if(b->page > 0) b->page--;
    } else if(ch == '.') {
        if((b->page + 1) * CHINESE_INPUT_CANDIDATES < b->len) {
            b->page++;
        }
    } else if(ch >= '1' && ch <= '9') {
        int i = b->pos + b->page * CHINESE_INPUT_CANDIDATES + ch - '1';
        if(i >= b->pos + b->len) return;

        uint16_t code = pinyin_data[i];
        uint8_t encode[3];
        if(code < 0x800) return;    // 2 bytes code, doesn't need to be supported
        // UTF-8 3 bytes code: 1110xxxx 10xxxxxx 10xxxxxx
        encode[0] = 0xe0 | (code >> 12);
        encode[1] = 0x80 | ((code >> 6) & 0x3f);
        encode[2] = 0x80 | (code & 0x3f);

        if(terminals[displayed_terminal_id].keyboard_buffer_top + 3 >= KEYBOARD_BUFFER_SIZE) return;

        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top] = encode[0];
        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top + 1] = encode[1];
        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top + 2] = encode[2];
        terminals[displayed_terminal_id].keyboard_buffer_top += 3;

        putc(encode[0]);
        putc(encode[1]);
        putc(encode[2]);

        b->buf_len = 0;
        b->pos = b->len = b->page = 0;
    } else if(b->buf_len == CHINESE_INPUT_BUF_LEN) {
        return;
    } else {
        if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
        if(ch < 'a' || ch > 'z') return;
        b->buf[b->buf_len] = ch;
        b->buf_len++;
        chinese_input_search();
    }
    chinese_input_draw();
}

void chinese_input_search() {
    volatile chinese_input_buf_t* b = &terminals[displayed_terminal_id].chinese_input_buf;
    if(b->buf_len == 0) {
        b->pos = b->len = b->page = 0;
        return;
    }

    pinyin_index_t* ptr = pinyin_index;
    while(ptr->pinyin[0] != '\0'
            && 0 != strncmp((const char*) b->buf, ptr->pinyin, b->buf_len + 1)) {
        ptr++;
    }
    b->pos = ptr->pos;
    b->len = ptr->len;
    b->page = 0;
    // printf("PY %d %d\n", ptr->pos, ptr->len);
}

void chinese_input_draw_utf8_char(uint16_t x, uint16_t y, uint16_t code, uint8_t attr) {
    uint8_t encode[3];
    if(code < 0x800) return;    // 2 bytes code, doesn't need to be supported
    // UTF-8 3 bytes code: 1110xxxx 10xxxxxx 10xxxxxx
    encode[0] = 0xe0 | (code >> 12);
    encode[1] = 0x80 | ((code >> 6) & 0x3f);
    encode[2] = 0x80 | (code & 0x3f);
    // Transfer 3 bytes one by one
    qemu_vga_putc(x, y, encode[0],
        qemu_vga_get_terminal_color(attr),
        qemu_vga_get_terminal_color(attr >> 4));
    qemu_vga_putc(x, y, encode[1],
        qemu_vga_get_terminal_color(attr),
        qemu_vga_get_terminal_color(attr >> 4));
    qemu_vga_putc(x, y, encode[2],
        qemu_vga_get_terminal_color(attr),
        qemu_vga_get_terminal_color(attr >> 4));
}

void chinese_input_draw() {
    volatile chinese_input_buf_t* b = &terminals[displayed_terminal_id].chinese_input_buf;
    int i;
    // Draw　user input section
    for(i = 0; i < CHINESE_INPUT_BUF_LEN; i++) {
        qemu_vga_putc(i * FONT_ACTUAL_WIDTH,
            CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
            (i < b->buf_len) ? b->buf[i] : ' ',
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_BUF),
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_BUF >> 4));
    }
    // Draw candidate section
    for(i = 0; i < CHINESE_INPUT_CANDIDATES; i++) {
        int x = CHINESE_INPUT_BUF_LEN + CHINESE_INPUT_CANDIDATE_WIDTH * i;
        qemu_vga_putc(x * FONT_ACTUAL_WIDTH,
            CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
            '1' + i,
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE),
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE >> 4));
        qemu_vga_putc((x + 1) * FONT_ACTUAL_WIDTH,
            CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
            '.',
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE),
            qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE >> 4));
        if(b->page * CHINESE_INPUT_CANDIDATES + i < b->len) {
            // There is a candidate character here
            chinese_input_draw_utf8_char((x + 2) * FONT_ACTUAL_WIDTH,
                CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
                pinyin_data[b->pos + b->page * CHINESE_INPUT_CANDIDATES + i],
                CHINESE_INPUT_ATTR_CANDIDATE);
        } else {
            qemu_vga_putc((x + 2) * FONT_ACTUAL_WIDTH,
                CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
                ' ',
                qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE),
                qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE >> 4));
            qemu_vga_putc((x + 3) * FONT_ACTUAL_WIDTH,
                CHINESE_INPUT_Y * FONT_ACTUAL_HEIGHT,
                ' ',
                qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE),
                qemu_vga_get_terminal_color(CHINESE_INPUT_ATTR_CANDIDATE >> 4));
        }
    }
}
