#include "mouse.h"
#include "i8259.h"
#include "keyboard.h"

int32_t mouse_x_cumulative = 0, mouse_y_cumulative = 0;
uint8_t mouse_left = 0, mouse_right = 1;

void mouse_reg_wait_out() {
    int i = 100000;
    while(i-- && inb(MOUSE_REG_PS2) & 2);
}

void mouse_reg_wait_in() {
    int i = 100000;
    while(i-- && inb(MOUSE_REG_PS2) & 1);
}

void mouse_reg_write(uint8_t data) {
    mouse_reg_wait_out();
    outb(MOUSE_CMD_SEND, MOUSE_REG_PS2);
    mouse_reg_wait_out();
    outb(data, MOUSE_REG_KEYBOARD);
}

uint8_t mouse_reg_read() {
    mouse_reg_wait_in();
    return inb(MOUSE_REG_KEYBOARD);
}

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

void mouse_interrupt() {
    mouse_t mouse;
    mouse.val = inb(MOUSE_REG_KEYBOARD);
    int8_t mouse_x = inb(MOUSE_REG_KEYBOARD);
    int8_t mouse_y = inb(MOUSE_REG_KEYBOARD);
    if(mouse.y_overflow || mouse.x_overflow || !mouse.reserved) {
        send_eoi(MOUSE_IRQ);
        return;
    }

    mouse_x_cumulative = mouse_x;
    mouse_y_cumulative = mouse_y;
    printf("mouse %c%c %d %d\n", mouse.btn_left ? 'L' : ' ', mouse.btn_right ? 'R' : ' ',
        mouse_x_cumulative, mouse_y_cumulative);
    send_eoi(MOUSE_IRQ);
}
