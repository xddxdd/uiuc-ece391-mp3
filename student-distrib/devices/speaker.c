#include "speaker.h"

/* void speaker_init()
 * @description: placeholder, currently does nothing.
 */
void speaker_init() {
    // Do nothing
}

/* void speaker_play(uint16_t hz)
 * @input: hz - hertz of sound to be played
 * @output: speaker plays the sound
 * @description: plays sound of specified frequency
 */
void speaker_play(uint16_t hz) {
    uint16_t div = PIT_FREQUENCY / hz;
    outb(0xb6, 0x43);
    outb((uint8_t) div, 0x42);
    outb((uint8_t) div >> 8, 0x42);
}

/* void speaker_play(uint16_t hz)
 * @input: row, tune - value of hertz in tune table.
 *                     see tune table at data/note-freq.c
 * @output: speaker plays the sound
 * @description: plays sound of specified frequency
 */
void speaker_tune(uint8_t row, tune_t tune) {
    printf("%d\n", note_tune(row, tune));
    speaker_play(note_tune(row, tune));
}

/* void speaker_mute()
 * @output: speaker stops playing sound
 * @description: stops speaker playing sound
 */
void speaker_mute() {
    uint8_t data = inb(0x61);
    data &= 0xfc;
    outb(data, 0x61);
}

/* void speaker_unmute()
 * @output: speaker continues playing sound
 * @description: let speaker continue playing sound
 */
void speaker_unmute() {
    uint8_t data = inb(0x61);
    data |= 0x03;
    outb(data, 0x61);
}
