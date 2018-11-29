#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include "../lib/lib.h"
#include "../data/note-freq.h"

#define PIT_FREQUENCY 1193180
#define SPEAKER_PORT_CMD 0x43
#define SPEAKER_PORT_DATA 0x42
#define SPEAKER_PORT_CONF 0x61
#define SPEAKER_INIT 0xb6
#define SPEAKER_MASK_MUTE 0xfc
#define SPEAKER_MASK_UNMUTE 0x03

void speaker_init();
void speaker_play(uint16_t hz);
void speaker_tune(uint8_t row, tune_t tune);
void speaker_mute();
void speaker_unmute();

#endif
