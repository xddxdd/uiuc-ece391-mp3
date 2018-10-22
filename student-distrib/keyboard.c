#include "keyboard.h"

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

    printf("Received scancode %d\n", key);

    send_eoi(KEYBOARD_IRQ);
}
