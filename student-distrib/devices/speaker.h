#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include "../lib.h"
#include "../data/note-freq.h"

#define PIT_FREQUENCY 1193180

void speaker_init();
void speaker_play(uint16_t hz);
void speaker_tune(uint8_t row, tune_t tune);
void speaker_mute();
void speaker_unmute();

#endif
