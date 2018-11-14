#include "keyboard.h"
#include "../data/keyboard-scancode.h"

// Unified FS interface definition for STDIN.
unified_fs_interface_t terminal_stdin_if = {
    .open = terminal_open,
    .read = terminal_read,
    .write = NULL,
    .close = NULL
};

// Unified FS interface definition for STDOUT.
unified_fs_interface_t terminal_stdout_if = {
    .open = terminal_open,
    .read = NULL,
    .write = terminal_write,
    .close = NULL
};

// keyboard buffer, one addition place for newline character
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE + 1];
// next available index in (i.e. top of) the keyboard buffer
// (default: 0 -- empty)
static int keyboard_buffer_top = 0;
// flag variable used to indicate whether writting to the keyborad buffer
// is enable or not (default: 0 -- disable)
static volatile int keyboard_buffer_enable = 0;

// Keyboard flags, tracking whether shift, ctrl, capslock are pressed or not.
// Added by jinghua3.
uint8_t shift_pressed = 0;
uint8_t ctrl_pressed = 0;
uint8_t capslock_pressed = 0;
uint8_t capslock = 0;

/* void keyboard_init()
 * @effects: Make the system ready to receive keyboard interrupts
 * @description: Enable the keyboard IRQ so that we can receive its interrupts
 */
void keyboard_init() {
    cli();
    enable_irq(KEYBOARD_IRQ);
    sti();
}

/* void keyboard_interrupt()
 * @input: PORT(KEYBOARD_PORT) - scancode sent from keyboard.
 * @description: Handle keyboard interrupt
 */
void keyboard_interrupt() {
    uint8_t scancode_idx = inb(KEYBOARD_PORT);
    char key;
    int is_special_key;

    is_special_key = update_special_key_stat(scancode_idx);
    if (is_special_key == 1){
        // send End Of Interrupt
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    if(ctrl_pressed==1){
        key = scancode[scancode_idx][0];
        if(key == 'l'){
            // Ctrl+L or Ctrl+l received, clear screen and put cursor at the top.
            clear();
            // clear the keyboard buffer.
            keyboard_buffer_top = 0;
        }

        // send End Of Interrupt
        send_eoi(KEYBOARD_IRQ);
        return;
    }
    // echo the keyboard input to the screen
    if(scancode_idx < SCANCODE_TABLE_SIZE) {
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
        if (keyboard_buffer_enable == 1) {
            if (key == BACKSPACE) {
                if(keyboard_buffer_top > 0) {
                    keyboard_buffer_top--;
                    cli();
                    keyboard_echo(key);
                    sti();
                }
            } else if (key == '\n') {
                cli();
                keyboard_echo(key);
                sti();
                // put newline character
                keyboard_buffer[keyboard_buffer_top] = '\n';
                // increment keyboard_buffer_top
                keyboard_buffer_top++;
                // disable keyboard buffer
                keyboard_buffer_enable = 0;
            } else if (keyboard_buffer_top >= KEYBOARD_BUFFER_SIZE) {
                cli();
                keyboard_echo('\n');
                sti();
                // put newline character
                keyboard_buffer[keyboard_buffer_top] = '\n';
                // increment keyboard_buffer_top
                keyboard_buffer_top++;
                // disable keyboard buffer
                keyboard_buffer_enable = 0;
            } else {
                cli();
                keyboard_echo(key);
                sti();
                // record current key
                keyboard_buffer[keyboard_buffer_top] = key;
                // increment keyboard_buffer_top
                keyboard_buffer_top++;
            }
        } else {
            cli();
            keyboard_echo(key);
            sti();
        }
    }
    // send End Of Interrupt
    send_eoi(KEYBOARD_IRQ);
}

/* int32_t terminal_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: SUCCESS
 * @description: prepare terminal for printing characters.
 */
int32_t terminal_open(int32_t* inode, char* filename)
{
    // clear the buffer
    keyboard_buffer_top = 0;
    // disable keyboard buffer
    keyboard_buffer_enable = 0;
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
    keyboard_buffer_enable = 1;

    /* printf("keyboard_read starts\n"); */
    // wait for keyboard inputs
    while (keyboard_buffer_enable == 1) {}
    /* printf("keyboard_read ends\n"); */
    for (index = 0; index < len; index++)
    {
        if (index < keyboard_buffer_top)
        {
            *(uint8_t *)(buf + index) = keyboard_buffer[index];
        }
        else
        {
            *(uint8_t *)(buf + index) = 0;
        }
    }
    min_size = keyboard_buffer_top < len ? keyboard_buffer_top : len;
    keyboard_buffer_top = 0;
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
        if (*(uint8_t *)(buf + index) == 0) break;
        putc(*(uint8_t *)(buf +index));
    }
    return index;
}

/* int32_t terminal_close(int32_t* inode)
 * @input: all ignored
 * @output: ret val - SUCCESS
 * @description: close terminal.
 */
int32_t terminal_close(int32_t* inode)
{
    // clear the buffer
    keyboard_buffer_top = 0;
    // disable keyboard buffer
    keyboard_buffer_enable = 0;
    return 0;
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
