#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "../lib/lib.h"
#include "i8259.h"
#include "vga_text.h"
#include "../fs/unified_fs.h"
#include "../interrupts/sys_calls.h"

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
#define LEFT_ALT_PRESS      0x38
#define LEFT_ALT_RELEASE    0xB8
#define LEFT_CTRL_PRESS     0x1D
#define LEFT_CTRL_RELEASE   0x9D

#define SCANCODE_F1         0x3B
#define SCANCODE_F2         0x3C
#define SCANCODE_F3         0x3D

void keyboard_init();
void keyboard_interrupt();

int update_special_key_stat(uint8_t keyboard_input);
int is_alphabet(uint8_t scancode);

// terminal Driver
int32_t terminal_open(int32_t* inode, char* filename);
int32_t terminal_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len);
int32_t terminal_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len);

extern unified_fs_interface_t terminal_stdin_if;
extern unified_fs_interface_t terminal_stdout_if;

#endif
