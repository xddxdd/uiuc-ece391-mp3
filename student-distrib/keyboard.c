#include "keyboard.h"
#include "data/keyboard-scancode.h"

// 1 additional space for newline character
static uint8_t keyboard_buffer[MAX_CHAR_NUM + 1];
// next available index in the keyboard buffer
static int keyboard_buffer_index;

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
    uint8_t key = inb(KEYBOARD_PORT);

    // disable putc, deletted by Zhenbang Wu
    // if(key < SCANCODE_TABLE_SIZE) {
        // putc(scancode[key][0]);
    // }

    // check the availability of the keyboard buffer
    if (keyboard_buffer_index < MAX_CHAR_NUM)
    {
      // record current key
      keyboard_buffer[keyboard_buffer_index] = scancode[key][0];
      // increment keyboard_buffer_index
      keyboard_buffer_index++;
      // temporarily add a newline character
      keyboard_buffer[keyboard_buffer_index] = '\n';
    }
    // send End Of Interrupt
    send_eoi(KEYBOARD_IRQ);
}


/* Keyboard Driver */
/* keyboard_open */
void keyboard_open()
{
  // clear the buffer
  keyboard_buffer_index = 0;
  // nothing in the buffer
  keyboard_buffer[0] = '\n';
  return;
}

/* keyboard_read */
void keyboard_read()
{
  // todo
  return;
}

/* keyboard_write */
void keyboard_write()
{
  int index;
  // if the keyboard buffer is not empty
  if (keyboard_buffer_index != 0)
  {
    for (index = 0; index <= keyboard_buffer_index; index++)
    {
        putc(keyboard_buffer[index]);
    }
  }
  keyboard_buffer_index = 0;
  keyboard_buffer[0] = '\n';
  return;
}

/* keyboard_close */
void keyboard_close()
{
  // there is nothing to do with syscall close() to keyboard
  return;
}
