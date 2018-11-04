#include "tux.h"
#include "serial.h"
#include "tux-mtcp.h"

unified_fs_interface_t tux_if = {
    .open = tux_open,
    .read = tux_read,
    .write = tux_write,
    .close = tux_close
};

#define TC_SERIAL_PORT COM1

// Initialization sequence of Tux Controller
#define TC_INITIALIZATION_SEQUENCE_LEN 2
const unsigned char tc_initialization_sequence[TC_INITIALIZATION_SEQUENCE_LEN] = {
    MTCP_BIOC_ON,   // Enable button interrupt
    MTCP_LED_USR    // Set LED content to accept user-defined content
};

// Length of LED control sequence
#define TC_LED_SEQUENCE_LEN 6
#define TC_LED_COUNT 4
#define TC_LED_OFFSET (TC_LED_SEQUENCE_LEN - TC_LED_COUNT)
uint8_t tc_led_sequence[TC_LED_SEQUENCE_LEN] = {MTCP_LED_SET, 0x0F, 0, 0, 0, 0};

// Variable used to hold state of buttons
volatile uint8_t tc_buttons = 0;

/* tc_led_segments: segment information for Tux Controller LED,
 *      maps 0-9 & A-Z to LED segment packet.
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

/* int8_t tux_init()
 * @output: SUCCESS / FAIL
 * @description: Initializes Tux Controller.
 */
int8_t tux_init() {
    if(SERIAL_OP_SUCCESS != serial_init(TC_SERIAL_PORT, TC_SERIAL_BAUDRATE)) return TUX_OP_FAIL;
    cli();
    int i;
    for(i = 0; i < TC_INITIALIZATION_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_initialization_sequence[i])) {
            sti();
            return TUX_OP_FAIL;
        }
    }
    for(i = 0; i < TC_LED_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_led_sequence[i])) {
            sti();
            return TUX_OP_FAIL;
        }
    }
    sti();
    return TUX_OP_SUCCESS;
}

/* int8_t tux_set_led(char* word, uint8_t dot)
 * @input: word - 4 character word to be displayed on LED
 *         dot - the least 4 bits record whether dots on LEDs will be on
 * @output: SUCCESS / FAIL
 * @description: Changes the content on display of Tux Controller.
 */
int8_t tux_set_led(char* word, uint8_t dot) {
    int i;
    char ch;
    for(i = 0; i < TC_LED_COUNT; i++) {
        ch = word[TC_LED_COUNT - i - 1];
        // Convert character to LED segment layout
        if(ch >= '0' && ch <= '9') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - '0')];
        } else if(ch >= 'A' && ch <= 'Z') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - 'A' + 10)];
        } else if(ch >= 'a' && ch <= 'z') {
            tc_led_sequence[TC_LED_OFFSET + i] = tc_led_segments[(int) (ch - 'a' + 10)];
        } else if(ch == ' ') {
            tc_led_sequence[TC_LED_OFFSET + i] = 0;
        } else {
            return TUX_OP_FAIL;
        }
        // If dot is enabled, set bit 4 for the layout, as described above
        if(dot & (1 << i)) {
            tc_led_sequence[TC_LED_OFFSET + i] |= 0x10;
        }
    }
    // Send the sequence to Tux Controller
    for(i = 0; i < TC_LED_SEQUENCE_LEN; i++) {
        if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_led_sequence[i])) {
            return TUX_OP_FAIL;
        }
    }
    return TUX_OP_SUCCESS;
}

/* void tux_interrupt(char packet)
 * @input: packet - new packet received, by the serial handler.
 *         tux_packet_buf - buffer of packets, holds up to 3,
 *             used for handling button press events.
 * @output: tc_buttons - updated to reflect button state
 *          tux_packet_buf - a new packet inserted
 *          Tux may get re-initialized if MTCP_RESET received
 * @description: Tux Controller interrupt handler.
 */
#define TUX_PACKET_BUF_LEN 3
char tux_packet_buf[TUX_PACKET_BUF_LEN] = {0, 0, 0};
void tux_interrupt(char packet) {
    // Move forward buf for handling button events
    // As we only cache 3 chars, we just use the simple way
    // of copying 3 times to implement a ring buffer.
    tux_packet_buf[0] = tux_packet_buf[1];
    tux_packet_buf[1] = tux_packet_buf[2];
    tux_packet_buf[2] = packet;
    // Check if the packet is the last of button event if:
    // - first packet is MTCP_BIOC_EVENT
    // - 2nd and 3rd packets' most significant bit is 1
    // See mp2-tux.pdf provided in mp2 for more info.
    if(tux_packet_buf[0] == MTCP_BIOC_EVENT
        && tux_packet_buf[1] & 0x80
        && tux_packet_buf[2] & 0x80) {
        // This packet is part of the 3 packets of button event
        tc_buttons = 0x00                       // Placeholder, make the code look cleaner
            | (tux_packet_buf[1] & 0x0F)        // Start, A, B, C button, bit 0-3 of pkt 1
            | ((tux_packet_buf[2] & 0x09) << 4) // Up, Right button, bit 0, 3 of pkt 2
            | ((tux_packet_buf[2] & 0x02) << 5) // Left button, bit 1 of pkt 2
            | ((tux_packet_buf[2] & 0x04) << 3);// Down button, bit 2 of pkt 2
        tc_buttons = ~tc_buttons;               // Flip the states for specs compliance
    } else if(tux_packet_buf[2] == MTCP_RESET) {
        // Tux reseted, resend initialization sequence & led sequence
        int i;
        for(i = 0; i < TC_INITIALIZATION_SEQUENCE_LEN; i++) {
            if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_initialization_sequence[i])) {
                return;
            }
        }
        for(i = 0; i < TC_LED_SEQUENCE_LEN; i++) {
            if(SERIAL_OP_SUCCESS != serial_write(TC_SERIAL_PORT, tc_led_sequence[i])) {
                return;
            }
        }
    }
}

/* int32_t tux_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: tux initialized
 * @description: enable tux controller.
 */
int32_t tux_open(int32_t* inode, char* filename) {
    return tux_init();
}

/* int32_t tux_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: buf - where tux's button state to be written to
 *         len - length of buf, should be sizeof(uint8_t)
 * @output: ret val - SUCCESS / FAIL
 *          buf - written with tux's button state
 * @description: copy tux's button state into buf, total 8 bits.
 *     Button bit relationship from 0 to 7: START, A, B, C, UP, DOWN, LEFT, RIGHT
 */
int32_t tux_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len) {
    if(buf == NULL) return TUX_OP_FAIL;
    if(len != sizeof(uint8_t)) return TUX_OP_FAIL;
    *buf = tc_buttons;
    return TUX_OP_SUCCESS;
}

/* int32_t tux_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len)
 * @input: buf - text (and decimal points) to be displayed on tux's LED
 *         len - length of buf, should be in range 1-5
 * @output: ret val - SUCCESS / FAIL
 *          tux controller's LED set with the given string
 * @description: set Tux Controller's LED to given string.
 *     If length in range 1-4, only text is displayed. If length < 4, the string is automatically
 *       padded to length 4 with spaces.
 *     If length is 5, first 4 characters are interpreted as text and displayed. The 5th byte's low 4 bits
 *       will be used to control which dots to be enabled.
 */
int32_t tux_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len) {
    char word[TC_LED_COUNT];
    uint8_t dot;

    if(buf == NULL) return TUX_OP_FAIL;
    if(len == 0 || len > TC_LED_COUNT + 1) return TUX_OP_FAIL;

    int i;
    for(i = 0; i < TC_LED_COUNT; i++) {
        word[i] = i < len ? buf[i] : ' ';
    }
    if(len == TC_LED_COUNT + 1) {
        dot = (uint8_t) buf[TC_LED_COUNT];
    }
    return tux_set_led(word, dot);
}

/* int32_t tux_close(int32_t* inode)
 * @input: all ignored
 * @output: ret val - SUCCESS
 * @description: closes Tux Controller, currently does nothing.
 */
int32_t tux_close(int32_t* inode) {
    return TUX_OP_SUCCESS;
}
