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
    // Speaker is driven by PIT, and PIT uses frequency divider
    // We calculate the interval to instruct PIT to generate a frequency
    uint16_t div = PIT_FREQUENCY / hz;
    outb(SPEAKER_INIT, SPEAKER_PORT_CMD);
    outb((uint8_t) div, SPEAKER_PORT_DATA);
    outb((uint8_t) div >> 8, SPEAKER_PORT_DATA);
}

/* void speaker_play(uint16_t hz)
 * @input: row, tune - value of hertz in tune table.
 *                     see tune table at data/note-freq.c
 * @output: speaker plays the sound
 * @description: plays sound of specified frequency
 */
void speaker_tune(uint8_t row, tune_t tune) {
    // printf("%d\n", note_tune(row, tune));
    speaker_play(note_tune(row, tune));
}

/* void speaker_mute()
 * @output: speaker stops playing sound
 * @description: stops speaker playing sound
 */
void speaker_mute() {
    uint8_t data = inb(SPEAKER_PORT_CONF);
    data &= SPEAKER_MASK_MUTE;
    outb(data, SPEAKER_PORT_CONF);
}

/* void speaker_unmute()
 * @output: speaker continues playing sound
 * @description: let speaker continue playing sound
 */
void speaker_unmute() {
    uint8_t data = inb(SPEAKER_PORT_CONF);
    data |= SPEAKER_MASK_UNMUTE;
    outb(data, SPEAKER_PORT_CONF);
}
