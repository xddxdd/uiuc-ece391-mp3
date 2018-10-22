#include "keyboard.h"
#include "keyboard-scancode.h"

/* void keyboard_init()
 * @effects: Make the system ready to receive keyboard interrupts
 * @description: Enable the keyboard IRQ so that we can receive its interrupts
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_interrupt()
 * @input: PORT(KEYBOARD_PORT) - scancode sent from keyboard.
 * @description: Prints user's key onto screen.
 */
void keyboard_interrupt() {
    uint8_t key = inb(KEYBOARD_PORT);

    if(key < SCANCODE_TABLE_SIZE) {
        putc(scancode[key][0]);
    }

    send_eoi(KEYBOARD_IRQ);
}
