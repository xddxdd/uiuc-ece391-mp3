#include "chinese_input.h"
#include "../devices/qemu_vga.h"
#include "../devices/keyboard.h"
#include "../interrupts/multiprocessing.h"

/* void chinese_input_keystroke(uint8_t ch)
 * @input: ch - keystroke
 * @output: updates multiple state of Chinese IME
 * @description: handles key event from keyboard interrupt handler.
 *     Should always be wrapped in ONTO_DISPLAY_WRAP.
 *     Supports typing, deleting (backspace), pageup/down (,/.),
 *     candidate choosing (1-9).
 */
void chinese_input_keystroke(uint8_t ch) {
    volatile chinese_input_buf_t* b = &terminals[displayed_terminal_id].chinese_input_buf;
    if(ch == BACKSPACE) {
        // Remove a character from IME buffer
        if(b->buf_len > 0) {
            b->buf_len--;
            b->buf[b->buf_len] = 0;
            // Trigger search of pinyin
            chinese_input_search();
        } else return;
    } else if(ch == ',') {
        // Switch to previous page if possible
        if(b->page > 0) b->page--;
    } else if(ch == '.') {
        // Switch to next page if possible
        if((b->page + 1) * CHINESE_INPUT_CANDIDATES < b->len) {
            b->page++;
        }
    } else if(ch >= '1' && ch <= '9') {
        // Selecting a candidate word, first check if the word is in range.
        int i = b->pos + b->page * CHINESE_INPUT_CANDIDATES + ch - '1';
        if(i >= b->pos + b->len) return;

        // Encode it to 3-byte UTF-8 encoding.
        uint16_t code = pinyin_data[i];
        uint8_t encode[3];
        if(code < 0x800) return;    // 2 bytes code, doesn't need to be supported
        // UTF-8 3 bytes code: 1110xxxx 10xxxxxx 10xxxxxx
        encode[0] = 0xe0 | (code >> 12);
        encode[1] = 0x80 | ((code >> 6) & 0x3f);
        encode[2] = 0x80 | (code & 0x3f);

        // Allocate 3 bytes in keyboard buffer, and copy the character in
        if(terminals[displayed_terminal_id].keyboard_buffer_top + 3 >= KEYBOARD_BUFFER_SIZE) return;

        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top] = encode[0];
        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top + 1] = encode[1];
        terminals[displayed_terminal_id].keyboard_buffer[terminals[displayed_terminal_id].keyboard_buffer_top + 2] = encode[2];
        terminals[displayed_terminal_id].keyboard_buffer_top += 3;

        // Also display it onto screen
        putc(encode[0]);
        putc(encode[1]);
        putc(encode[2]);

        // Clear IME state for next input
        b->buf_len = 0;
        b->pos = b->len = b->page = 0;
    } else if(b->buf_len == CHINESE_INPUT_BUF_LEN) {
        // Buffer is full, no more typing allowed
        return;
    } else {
        // Transform uppercase to undercase
        if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
        // Only allow letters
        if(ch < 'a' || ch > 'z') return;
        // Add letter into buf
        b->buf[b->buf_len] = ch;
        b->buf_len++;
        // Trigger search
        chinese_input_search();
    }
    // Trigger refresh of IME UI
    chinese_input_draw();
}

/* void chinese_input_search()
 * @input: chinese_input_buf - IME state of current displayed terminal.
 * @output: a list of characters is stored in the current state.
 * @description: searches for the characters that have the same pinyin
 *     as stored in buffer, and updates position, length, etc into IME state.
 */
void chinese_input_search() {
    volatile chinese_input_buf_t* b = &terminals[displayed_terminal_id].chinese_input_buf;
    // If input is empty, clear everything
    if(b->buf_len == 0) {
        b->pos = b->len = b->page = 0;
        return;
    }

    pinyin_index_t* ptr = pinyin_index;
    while(ptr->len != 0) {
        // Find the exact match of pinyin
        // (Fuzzy pinyin needs a lot more work, won't be implemented)
        if((strlen(ptr->pinyin) == b->buf_len)
            && (0 == strncmp((const char*) b->buf, ptr->pinyin, b->buf_len))) {
            // Found the pinyin in database
            b->pos = ptr->pos;
            b->len = ptr->len;
            b->page = 0;
            return;
        }
        ptr++;
    }
    // Nothing found, clear everything
    b->pos = b->len = b->page = 0;
}

/* void chinese_input_draw_utf8_char(uint16_t x, uint16_t y, uint16_t code, uint8_t attr)
 * @input: x, y - coordinate of the UTF-8 character
 *         code - 16 bit code (NOT 3-byte huffman) for character
 *         attr - foreground and background color attribute
 * @output: character printed with given settings
 * @description: prints a UTF-8 character at the given position.
 *     Used by IME redraw routine to draw the candidate characters.
 */
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

/* void chinese_input_draw()
 * @output: IME UI gets refreshed on current display
 * @description: redraws the bar of IME.
 *     Only QEMU VGA will use this area, so putc() isn't used.
 */
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
            // No candidate here, clear the position with two spaces
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
