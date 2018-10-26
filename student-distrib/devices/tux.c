#include "tux.h"
#include "serial.h"
#include "tux-mtcp.h"

#define TC_SERIAL_PORT COM1

#define TC_INITIALIZATION_SEQUENCE_LEN 2
const unsigned char tc_initialization_sequence[TC_INITIALIZATION_SEQUENCE_LEN] = {
    MTCP_BIOC_ON,   // Enable button interrupt
    MTCP_LED_USR    // Set LED content to accept user-defined content
};

#define TC_LED_SEQUENCE_LEN 6
#define TC_LED_COUNT 4
#define TC_LED_OFFSET (TC_LED_SEQUENCE_LEN - TC_LED_COUNT)
uint8_t tc_led_sequence[TC_LED_SEQUENCE_LEN] = {MTCP_LED_SET, 0x0F, 0, 0, 0, 0};

uint8_t tc_buttons = 0;

/* tc_led_segments: segment information for Tux Controller LED,
 *      maps 0-9 & A-F to LED segment packet.
 * Tux Controller segment to packet bit mapping:
 *   +--7--+
 * 5 |     | 1
 *   +--3--+
 * 6 |     | 2
 *   +--0--+ dot(4)
 * Hand calculated, you can calculate again to verify this
 */
const uint8_t tc_led_segments[] = {
    // 0,   1,    2,    3,    4,    5,    6,    7,    8,    9
    0xe7, 0x6, 0xcb, 0x8f, 0x2e, 0xad, 0xed, 0x86, 0xef, 0xaf,
    // A,    B,    C,    D,    E,    F,    G,    H,   I,    J
    0xee, 0x6d, 0x49, 0x4f, 0xe9, 0xe8, 0xe5, 0x6e, 0x6, 0x47,
    // K,    L,    M,    N,    O,    P,    Q,    R,    S,    T
    0x6e, 0x61, 0xe6, 0x4c, 0xe7, 0xea, 0xae, 0xee, 0xad, 0x69,
    // U,    V,    W,    X,    Y,    Z
    0x45, 0x67, 0x67, 0x6e, 0x2f, 0xcb
};

int8_t tux_init() {
    if(SERIAL_OP_SUCCESS != serial_init(TC_SERIAL_PORT)) return TUX_OP_FAIL;
    int i;
    for(i = 0; i < TC_INITIALIZATION_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_initialization_sequence[i])) {
            return TUX_OP_FAIL;
        }
    }
    for(i = 0; i < TC_LED_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_led_sequence[i])) {
            return TUX_OP_FAIL;
        }
    }
    return TUX_OP_SUCCESS;
}

int8_t tux_set_led(char* word, uint8_t dot) {
    int i;
    char ch;
    for(i = 0; i < TC_LED_COUNT; i++) {
        ch = word[TC_LED_COUNT - i - 1];
        if(ch >= '0' && ch <= '9') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - '0')];
        } else if(ch >= 'A' && ch <= 'Z') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - 'A' + 10)];
        } else if(ch >= 'a' && ch <= 'z') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - 'a' + 10)];
        } else {
            return TUX_OP_FAIL;
        }
        if(dot & (1 << i)) {
            tc_led_sequence[TC_LED_OFFSET + i] |= 0x10;
        }
    }
    for(i = 0; i < TC_LED_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_led_sequence[i])) {
            return TUX_OP_FAIL;
        }
    }
    return TUX_OP_SUCCESS;
}

void tux_interrupt() {

}
