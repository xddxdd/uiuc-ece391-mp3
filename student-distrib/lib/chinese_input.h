#ifndef _CHINESE_INPUT_H_
#define _CHINESE_INPUT_H_

#include "lib.h"
#include "../data/chinese_pinyin.h"

#define CHINESE_INPUT_Y 28
#define CHINESE_INPUT_BUF_LEN 10
#define CHINESE_INPUT_CANDIDATES 9
#define CHINESE_INPUT_CANDIDATE_WIDTH 5

typedef struct {
    char buf[CHINESE_INPUT_BUF_LEN + 1];
    uint8_t buf_len;// length of buffer
    uint16_t pos;   // position of pinyin in pinyin list
    uint16_t len;   // length of words in pinyin list
    uint16_t page;
} chinese_input_buf_t;

#define CHINESE_INPUT_ATTR_BUF 0x0e
#define CHINESE_INPUT_ATTR_CANDIDATE 0x0f

extern chinese_input_buf_t chinese_input_buf;
extern uint8_t chinese_input_enabled;

void chinese_input_keystroke(uint8_t ch);
void chinese_input_search();
void chinese_input_draw_utf8_char(uint16_t x, uint16_t y, uint16_t code, uint8_t attr);
void chinese_input_draw();

#endif
