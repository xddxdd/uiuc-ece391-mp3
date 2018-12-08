#ifndef _CHINESE_PINYIN_H_
#define _CHINESE_PINYIN_H_

#include "../lib/lib.h"

typedef struct {
    char pinyin[7];     // Longest pinyin is 7 characters
    uint16_t pos;       // Where the characters start
    uint16_t len;       // How many characters with this pinyin
} pinyin_index_t;

extern uint16_t pinyin_data[];
extern pinyin_index_t pinyin_index[];

#endif
