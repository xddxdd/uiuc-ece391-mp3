#include "note-freq.h"

// Note frequencies from https://www.seventhstring.com/resources/notefrequencies.html
uint16_t notes[] = {
    16,17,18,19,20,21,23,24,25,27,29,30,
    32,34,36,38,41,43,46,49,51,55,58,61,
    65,69,73,77,82,87,92,98,103,110,116,123,
    130,138,146,155,164,174,185,196,207,220,233,246,
    261,277,293,311,329,349,370,392,415,440,466,493,
    523,554,587,622,659,698,740,784,830,880,932,987,
    1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976,
    2093,2217,2349,2489,2637,2794,2960,3136,3322,3520,3729,3951,
    4186,4435,4699,4978,5274,5588,5920,6272,6645,7040,7459,7902,
};

/* uint16_t note_tune(uint8_t row, tune_t tune)
 * @input: row - the number of row in frequency table, e.g. 3 for C3
 *         tune - the number of column in frequency table, e.g. C for C3
 * @output: ret val - frequency of the note
 * @description: convert note to frequency for speaker playing
 */
uint16_t note_tune(uint8_t row, tune_t tune) {
    return notes[row * TUNE_TOTAL + tune];
}
