#include "keyboard.h"

void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

void keyboard_interrupt() {
    uint8_t key = inb(KEYBOARD_PORT);

    printf("Received scancode %d\n", key);

    send_eoi(KEYBOARD_IRQ);
}
