#include "mouse.h"
#include "i8259.h"
#include "keyboard.h"

volatile int32_t mouse_x_cumulative = 0, mouse_y_cumulative = 0;
volatile uint8_t mouse_left = 0, mouse_right = 0;
volatile uint8_t mouse_used = 0;

/* void mouse_reg_wait_out()
 * @description: wait until mouse can accept another packet.
 */
void mouse_reg_wait_out() {
    int i = 100000;
    while(i-- && inb(MOUSE_REG_PS2) & 2);
}

/* void mouse_reg_wait_in()
 * @description: wait until mouse can send another packet.
 */
void mouse_reg_wait_in() {
    int i = 100000;
    while(i-- && inb(MOUSE_REG_PS2) & 1);
}

/* void mouse_reg_write(uint8_t data)
 * @input: data - data to be sent to mouse
 * @description: send the data to mouse.
 */
void mouse_reg_write(uint8_t data) {
    mouse_reg_wait_out();
    outb(MOUSE_CMD_SEND, MOUSE_REG_PS2);
    mouse_reg_wait_out();
    outb(data, MOUSE_REG_KEYBOARD);
}

/* uint8_t mouse_reg_read()
 * @output: ret val - data from mouse
 * @description: receive a data from mouse
 */
uint8_t mouse_reg_read() {
    mouse_reg_wait_in();
    return inb(MOUSE_REG_KEYBOARD);
}

/* void mouse_init()
 * @description: initialize the mouse to send packets on move.
 */
void mouse_init() {
    mouse_reg_wait_out(); outb(MOUSE_CMD_AUXILARY, MOUSE_REG_PS2);
    mouse_reg_wait_out(); outb(MOUSE_GET_COMPAQ_STATUS, MOUSE_REG_PS2);
    mouse_reg_wait_in(); uint8_t status = inb(MOUSE_REG_KEYBOARD);
    status |= 2;    // Enable interrupt 12 bit
    status &= 0xdf; // Clear disable mouse clock bit
    mouse_reg_wait_out(); outb(MOUSE_SET_COMPAQ_STATUS, MOUSE_REG_PS2);
    mouse_reg_wait_out(); outb(status, MOUSE_REG_KEYBOARD);
    mouse_reg_write(MOUSE_CMD_DEFAULT);
    mouse_reg_read();
    mouse_reg_write(MOUSE_CMD_ENABLE_STREAM);
    mouse_reg_read();

    mouse_reg_wait_out();
    outb(MOUSE_CMD_SEND, MOUSE_REG_PS2);
    mouse_reg_wait_out();
    outb(MOUSE_CMD_SAMPLE_RATE, MOUSE_REG_KEYBOARD);
    mouse_reg_wait_out();
    outb(200, MOUSE_REG_KEYBOARD);

    enable_irq(MOUSE_IRQ);
}

/* void mouse_interrupt()
 * @description: mouse interrupt handler, updates mouse values
 */
void mouse_interrupt() {
    cli();
    mouse_t mouse;
    mouse.val = inb(MOUSE_REG_KEYBOARD);
    int8_t mouse_x = inb(MOUSE_REG_KEYBOARD);
    int8_t mouse_y = inb(MOUSE_REG_KEYBOARD);
    if(mouse.y_overflow || mouse.x_overflow || !mouse.reserved) {
        send_eoi(MOUSE_IRQ);
        sti();
        return;
    }

    mouse_x_cumulative = mouse_x;
    mouse_y_cumulative = mouse_y;
    mouse_left = mouse.btn_left;
    mouse_right = mouse.btn_right;
    // printf("mouse %c%c %d %d\n", mouse.btn_left ? 'L' : ' ', mouse.btn_right ? 'R' : ' ',
    //     mouse_x_cumulative, mouse_y_cumulative);
    sti();
    send_eoi(MOUSE_IRQ);
}

unified_fs_interface_t mouse_if = {
    .open = mouse_open,
    .read = mouse_read,
    .write = NULL,
    .ioctl = NULL,
    .close = mouse_close
};

/* int32_t mouse_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: 0 (SUCCESS)
 * @description: initialize mouse, clear all recorded data.
 */
int32_t mouse_open(int32_t* inode, char* filename) {
    cli();
    if(mouse_used) {
        sti();
        return FAIL;
    }
    mouse_used = 1;
    mouse_x_cumulative = 0;
    mouse_y_cumulative = 0;
    mouse_left = 0;
    mouse_right = 0;
    sti();
    return SUCCESS;
}

/* int32_t mouse_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @input: all ignored
 * @output: 0 (SUCCESS)
 * @description: read mouse status.
 */
int32_t mouse_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len) {
    if(buf == NULL) return FAIL;
    if(len < 4 * sizeof(int32_t)) return FAIL;

    cli();
    int32_t* ptr = (int32_t*) buf;
    ptr[0] = mouse_x_cumulative;
    ptr[1] = mouse_y_cumulative;
    ptr[2] = mouse_left;
    ptr[3] = mouse_right;

    mouse_x_cumulative = 0;
    mouse_y_cumulative = 0;
    sti();

    return SUCCESS;
}

/* int32_t mouse_close(int32_t* inode)
 * @input: inode - ignored
 * @output: 0 (SUCCESS)
 * @description: release mouse for use with other programs.
 */
int32_t mouse_close(int32_t* inode) {
    mouse_used = 0;
    return SUCCESS;
}
