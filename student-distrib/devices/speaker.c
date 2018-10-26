#include "speaker.h"

void speaker_init() {
    // Do nothing
}

void speaker_play(uint16_t hz) {
    uint16_t div = PIT_FREQUENCY / hz;
    outb(0xb6, 0x43);
    outb((uint8_t) div, 0x42);
    outb((uint8_t) div >> 8, 0x42);
}

void speaker_tune(uint8_t row, tune_t tune) {
    printf("%d\n", note_tune(row, tune));
    speaker_play(note_tune(row, tune));
}

void speaker_mute() {
    uint8_t data = inb(0x61);
    data &= 0xfc;
    outb(data, 0x61);
}

void speaker_unmute() {
    uint8_t data = inb(0x61);
    data |= 0x03;
    outb(data, 0x61);
}
