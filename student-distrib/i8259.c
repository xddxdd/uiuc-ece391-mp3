/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    outb(ICW1, MASTER_8259_CMD);
    outb(ICW1, SLAVE_8259_CMD);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    enable_irq(SLAVE_8259_PORT);
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return;
    uint8_t change_mask = 0;
    if(irq_num >= I8259_PORT_COUNT) {
        change_mask = 1 << (irq_num - I8259_PORT_COUNT);
        slave_mask &= ~change_mask;
        outb(slave_mask, SLAVE_8259_DATA);
    } else {
        change_mask = 1 << irq_num;
        master_mask &= ~change_mask;
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return;
    uint8_t change_mask = 0;
    if(irq_num >= I8259_PORT_COUNT) {
        change_mask = 1 << (irq_num - I8259_PORT_COUNT);
        slave_mask |= change_mask;
        outb(slave_mask, SLAVE_8259_DATA);
    } else {
        change_mask = 1 << irq_num;
        master_mask |= change_mask;
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return;
    uint8_t data = EOI;
    if(irq_num > I8259_PORT_COUNT) {
        data |= irq_num - 8;
        outb(data, SLAVE_8259_CMD);
        send_eoi(SLAVE_8259_PORT);
    } else {
        data |= irq_num;
        outb(data, MASTER_8259_CMD);
    }
}
