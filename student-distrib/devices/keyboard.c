#include "keyboard.h"
#include "../interrupts/multiprocessing.h"
#include "../data/keyboard-scancode.h"
#include "../lib/chinese_input.h"
#include "../devices/qemu_vga.h"

// Unified FS interface definition for STDIN.
unified_fs_interface_t terminal_stdin_if = {
    .open = terminal_open,
    .read = terminal_read,
    .write = NULL,
    .ioctl = NULL,
    .close = NULL
};

// Unified FS interface definition for STDOUT.
unified_fs_interface_t terminal_stdout_if = {
    .open = terminal_open,
    .read = NULL,
    .write = terminal_write,
    .ioctl = NULL,
    .close = NULL
};

// Keyboard flags, tracking whether shift, ctrl, capslock are pressed or not.
// Added by jinghua3.
uint8_t shift_pressed = 0;
uint8_t ctrl_pressed = 0;
uint8_t alt_pressed = 0;
uint8_t capslock_pressed = 0;
uint8_t capslock = 0;

uint8_t ctrl_c_pending = 0;

/* void keyboard_init()
 * @effects: Make the system ready to receive keyboard interrupts
 * @description: Enable the keyboard IRQ so that we can receive its interrupts
 */
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/* void keyboard_interrupt()
 * @input: PORT(KEYBOARD_PORT) - scancode sent from keyboard.
 * @description: Handle keyboard interrupt
 */
void keyboard_interrupt() {
    cli();
    terminal_switch_active(displayed_terminal_id);

    uint8_t scancode_idx = inb(KEYBOARD_PORT);
    char key;
    int is_special_key;

    volatile terminal_t* t = &terminals[displayed_terminal_id];

    is_special_key = update_special_key_stat(scancode_idx);
    if (is_special_key == 1){
        // send End Of Interrupt
        send_eoi(KEYBOARD_IRQ);
        sti();
        return;
    }

    if(ctrl_pressed==1){
        key = scancode[scancode_idx][0];
        if(key == 'l'){
            // Ctrl+L or Ctrl+l received, clear screen and put cursor at the top.
            ONTO_DISPLAY_WRAP(clear());
            // clear the keyboard buffer.
            t->keyboard_buffer_top = 0;
        }

        // send End Of Interrupt
        send_eoi(KEYBOARD_IRQ);

        if(key == 'c') {
            // Ctrl+C received, schedule killing current process
            if(displayed_terminal_id == active_terminal_id) {
                syscall_halt(255);  // 255 is return code, indicate that process exited abnormally
            } else {
                ctrl_c_pending = 1;
            }
        }
        sti();
        return;
    } else if(alt_pressed == 1) {
        send_eoi(KEYBOARD_IRQ);
        if(scancode_idx == SCANCODE_F1) {
            terminal_switch_display(0);
        } else if(scancode_idx == SCANCODE_F2) {
            terminal_switch_display(1);
        } else if(scancode_idx == SCANCODE_F3) {
            terminal_switch_display(2);
        }
        sti();
        return;
    } else if(scancode_idx < SCANCODE_TABLE_SIZE) {
        // Caps check, modified by jinghua3.
        if(((capslock) != (shift_pressed)) && is_alphabet(scancode_idx)){
          key = scancode[scancode_idx][1];
        }
        else if(!is_alphabet(scancode_idx) && shift_pressed){
          key = scancode[scancode_idx][1];
        }
        else{
          key = scancode[scancode_idx][0];
        }

        // if keyboard buffer is enable
        if (t->keyboard_buffer_enable == 1) {
            if(t->chinese_input_buf.enabled
                && key != '\n'
                && (key != BACKSPACE || t->chinese_input_buf.buf_len > 0)) {
                ONTO_DISPLAY_WRAP(chinese_input_keystroke(key));
            } else if (key == BACKSPACE) {
                if(t->keyboard_buffer_top > 0) {
                    int i = t->keyboard_buffer_top - 1;
                    if(t->keyboard_buffer[i] & UTF8_MASK) {
                        // This is part of a UTF-8 code, find its beginning
                        while(((t->keyboard_buffer[i] & UTF8_2BYTE_MASK) != UTF8_2BYTE_MASK)
                            && (t->keyboard_buffer_top - i <= 3)) i--;
                        if((t->keyboard_buffer[i] & UTF8_2BYTE_MASK) == UTF8_2BYTE_MASK) {
                            // We found the beginning of the code
                            t->keyboard_buffer_top = i;
                            // Remove the 2 char wide character on screen
                            ONTO_DISPLAY_WRAP(putc(BACKSPACE));
                            ONTO_DISPLAY_WRAP(putc(BACKSPACE));
                        } else {
                            t->keyboard_buffer_top--;
                            ONTO_DISPLAY_WRAP(putc(BACKSPACE));
                        }
                    } else {
                        t->keyboard_buffer_top--;
                        ONTO_DISPLAY_WRAP(putc(BACKSPACE));
                    }
                }
            } else if (key == '\n') {
                ONTO_DISPLAY_WRAP(putc(key));
                // put newline character
                t->keyboard_buffer[t->keyboard_buffer_top] = '\n';
                // increment keyboard_buffer_top
                t->keyboard_buffer_top++;
                // disable keyboard buffer
                t->keyboard_buffer_enable = 0;
            } else if (t->keyboard_buffer_top >= KEYBOARD_BUFFER_SIZE) {
                // Prevent entering more keys

                // ONTO_DISPLAY_WRAP(putc('\n'));
                // // put newline character
                // t->keyboard_buffer[t->keyboard_buffer_top] = '\n';
                // // increment keyboard_buffer_top
                // t->keyboard_buffer_top++;
                // // disable keyboard buffer
                // t->keyboard_buffer_enable = 0;
            } else {
                ONTO_DISPLAY_WRAP(putc(key));
                // record current key
                t->keyboard_buffer[t->keyboard_buffer_top] = key;
                // increment keyboard_buffer_top
                t->keyboard_buffer_top++;
            }
        } else {
            ONTO_DISPLAY_WRAP(putc(key));
        }
    }
    // send End Of Interrupt
    send_eoi(KEYBOARD_IRQ);
    sti();
}

/* int32_t terminal_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: SUCCESS
 * @description: prepare terminal for printing characters.
 */
int32_t terminal_open(int32_t* inode, char* filename)
{
    // clear the buffer
    terminals[active_terminal_id].keyboard_buffer_top = 0;
    // disable keyboard buffer
    terminals[active_terminal_id].keyboard_buffer_enable = 0;

    return 0;
}

/* int32_t terminal_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: buf - buffer to be written to
 *         len - max length of string to be read from keyboard
 * @output: ret val - SUCCESS / FAIL
 *          buf - written with user's input
 * @description: read from keyboard input.
 */
int32_t terminal_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
{
    // invalid input
    if (buf == NULL)
    {
      return -1;
    }
    // loop index
    int index;
    // return value
    int min_size;
    // enable keyboard buffer
    terminals[active_terminal_id].keyboard_buffer_enable = 1;

    /* printf("keyboard_read starts\n"); */
    // wait for keyboard inputs
    while (terminals[active_terminal_id].keyboard_buffer_enable == 1);
    /* printf("keyboard_read ends\n"); */
    cli();
    for (index = 0; index < len; index++)
    {
        if (index < terminals[active_terminal_id].keyboard_buffer_top)
        {
            *(uint8_t *)(buf + index) = terminals[active_terminal_id].keyboard_buffer[index];
        }
        else
        {
            *(uint8_t *)(buf + index) = 0;
        }
    }
    min_size = terminals[active_terminal_id].keyboard_buffer_top < len
             ? terminals[active_terminal_id].keyboard_buffer_top : len;
    terminals[active_terminal_id].keyboard_buffer_top = 0;
    sti();
    return min_size;
}

/* int32_t terminal_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len)
 * @input: buf - buffer to be read from
 *         len - length of characters to be printed on screen
 * @output: ret val - SUCCESS / FAIL
 * @description: print characters onto the screen.
 */
int32_t terminal_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len)
{
    // invalid input
    if (buf == NULL)
    {
      return -1;
    }
    // loop index
    int index;
    // all data in the buffer are displayed to the screen
    for (index = 0; index < len; index++)
    {
        // if (*(uint8_t *)(buf + index) == 0) break;
        putc(*(uint8_t *)(buf +index));
    }
    return index;
}

/* update_special_key_stat - Added by jinghua3.
 *
 * update the status: (pressed/not pressed) of Shift, Ctrl, Capslock
 * INPUT:  data from keyboard port.
 * OUTPUT: updates special key status.
 * RETURN: 0 if the scancode from keyboard is not a special key like Shift, Ctrl and Capslock,
 *         otherwise 1.
 */
int update_special_key_stat(uint8_t keyboard_input){
  volatile terminal_t* t = &terminals[displayed_terminal_id];
  switch(keyboard_input){
    case CAPSLOCK_PRESS:
      capslock_pressed = 1;
      if (capslock==0){
        capslock = 1;
      }
      else{
        capslock = 0;
      }
      return 1;

    case CAPSLOCK_RELEASE:
      capslock_pressed = 0;
      return 1;

    case LEFT_SHIFT_PRESS:
    case RIGHT_SHIFT_PRESS:
      shift_pressed = 1;
      return 1;

    case LEFT_SHIFT_RELEASE:
    case RIGHT_SHIFT_RELEASE:
      shift_pressed = 0;
      // If QEMU VGA is enabled, Chinese IME can work, so toggle it
      if(qemu_vga_enabled) t->chinese_input_buf.enabled = 1 - t->chinese_input_buf.enabled;
      return 1;

    case LEFT_ALT_PRESS:
      alt_pressed = 1;
      return 1;
    case LEFT_ALT_RELEASE:
      alt_pressed = 0;
      return 1;

    case LEFT_CTRL_PRESS:
      ctrl_pressed = 1;
      return 1;
    case LEFT_CTRL_RELEASE:
      ctrl_pressed = 0;
      return 1;

    default:
      return 0;
  }
}

/* is_alphabet - Added by jinghua3.
 *
 * check if keyboard input is an alphabet
 * INPUT: scancode from keyboard.
 * OUTPUT:none
 * RETURN: 1 if is alphabet, 0 if not.
 */

int is_alphabet(uint8_t scancode_idx){
  char key;
  if (scancode_idx < SCANCODE_TABLE_SIZE){
    key = scancode[scancode_idx][0];
  }
  else{
    return 0;
  }

  if(key>='a' && key<='z'){
    return 1;
  }
  else {
    return 0;
  }
}
