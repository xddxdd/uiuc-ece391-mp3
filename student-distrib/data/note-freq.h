#ifndef _NOTE_FREQ_H_
#define _NOTE_FREQ_H_

#include "../types.h"

typedef enum {
    TUNE_C = 0,
    TUNE_CS = 1,
    TUNE_D = 2,
    TUNE_EB = 3,
    TUNE_E = 4,
    TUNE_F = 5,
    TUNE_FS = 6,
    TUNE_G = 7,
    TUNE_GS = 8,
    TUNE_A = 9,
    TUNE_BB = 10,
    TUNE_B = 11,
    TUNE_TOTAL = 12
} tune_t;

uint16_t note_tune(uint8_t row, tune_t tune);

#endif
