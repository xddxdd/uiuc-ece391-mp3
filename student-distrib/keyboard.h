#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "lib.h"
#include "i8259.h"
#include "vga_text.h"

#define KEYBOARD_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 127    /* maximum number of characters in the buffer */
                                    /* except for the newline character */
#define BACKSPACE   0x8

void keyboard_init();
void keyboard_interrupt();

// Keyboard Driver
void keyboard_open();
void keyboard_read();
void keyboard_write();
void keyboard_close();

#endif
