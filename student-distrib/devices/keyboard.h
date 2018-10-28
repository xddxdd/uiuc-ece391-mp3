#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../lib.h"
#include "i8259.h"
#include "vga_text.h"

#define KEYBOARD_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 127    /* maximum number of characters in the buffer */
                                    /* except for the newline character */
#define BACKSPACE   0x8

// PS/2 keyboard input data
#define CAPSLOCK_PRESS      0x3A
#define CAPSLOCK_RELEASE    0xBA
#define LEFT_SHIFT_PRESS    0x2A
#define RIGHT_SHIFT_PRESS   0x36
#define LEFT_SHIFT_RELEASE  0xAA
#define RIGHT_SHIFT_RELEASE 0xB6
#define LEFT_CTRL_PRESS     0x1D
#define LEFT_CTRL_RELEASE   0x9D

void keyboard_init();
void keyboard_interrupt();

int update_special_key_stat(uint8_t keyboard_input);
int is_alphabet(uint8_t scancode);

// Keyboard Driver
void keyboard_open();
void keyboard_read();
void keyboard_write();
void keyboard_close();

#endif
