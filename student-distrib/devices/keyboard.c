#include "keyboard.h"
#include "../data/keyboard-scancode.h"

// keyboard buffer, one addition place for newline character
static uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE + 1];
// next available index in (i.e. top of) the keyboard buffer
// (default: 0 -- empty)
static int keyboard_buffer_top = 0;
// flag variable used to indicate whether writting to the keyborad buffer
// is enable or not (default: 0 -- disable)
static volatile int keyboard_buffer_enable = 0;

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
    uint8_t scancode_idx = inb(KEYBOARD_PORT);
    char key;

    /* Todo: add upper case support */

    // echo the keyboard input to the screen
    if(scancode_idx < SCANCODE_TABLE_SIZE)
    {
        key = scancode[scancode_idx][0];
        keyboard_echo(key);
        // if keyboard buffer is enable
        if (keyboard_buffer_enable == 1)
        {
          if (key == BACKSPACE)
          {
            keyboard_buffer_top = (keyboard_buffer_top - 1) < 0 ? 0 :  keyboard_buffer_top - 1;
          }
          else if (key == '\n')
          {
            // put newline character
            keyboard_buffer[keyboard_buffer_top] = '\n';
            // increment keyboard_buffer_top
            keyboard_buffer_top++;
            // disable keyboard buffer
            keyboard_buffer_enable = 0;
          }
          else if (keyboard_buffer_top >= KEYBOARD_BUFFER_SIZE)
          {
            keyboard_echo('\n');
            // put newline character
            keyboard_buffer[keyboard_buffer_top] = '\n';
            // increment keyboard_buffer_top
            keyboard_buffer_top++;
            // disable keyboard buffer
            keyboard_buffer_enable = 0;
          }
          else
          {
            // record current key
            keyboard_buffer[keyboard_buffer_top] = key;
            // increment keyboard_buffer_top
            keyboard_buffer_top++;
          }
        }
    }
    // send End Of Interrupt
    send_eoi(KEYBOARD_IRQ);
}


/* Keyboard Driver */
/* keyboard_open */
void keyboard_open()
{
  // clear the buffer
  keyboard_buffer_top = 0;
  // disable keyboard buffer
  keyboard_buffer_enable = 0;
  return;
}

/* keyboard_read */
void keyboard_read()
{
  // enable keyboard buffer
  keyboard_buffer_enable = 1;
  /* printf("keyboard_read starts\n"); */
  // wait for keyboard inputs
  while (keyboard_buffer_enable == 1) {}
  /* printf("keyboard_read ends\n"); */
  return;
}

/* keyboard_write */
void keyboard_write()
{
  // loop index
  int index;
  // all data in the buffer are displayed to the screen
  for (index = 0; index < keyboard_buffer_top; index++)
  {
      putc(keyboard_buffer[index]);
  }
  keyboard_buffer_top = 0;
  return;
}

/* keyboard_close */
void keyboard_close()
{
  // clear the buffer
  keyboard_buffer_top = 0;
  // disable keyboard buffer
  keyboard_buffer_enable = 0;
  return;
}
