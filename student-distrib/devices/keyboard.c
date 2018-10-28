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
    enable_irq(KEYBOARD_IRQ);
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
    if(scancode_idx < SCANCODE_TABLE_SIZE)
    {
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