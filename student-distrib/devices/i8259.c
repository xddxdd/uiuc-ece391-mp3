/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "../lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* void i8259_init()
 * @effects: Master i8259 PIC set to generate interrupt 0x20-0x27,
 *           Slave i8259 PIC set to generate interrupt 0x28-0x2F,
 *           Slave PIC attached to line 2 of Master PIC
 * @description: Initialize both PICs for handling interrupts.
 */
void i8259_init(void) {
    // The given sequence to initialize both PICs, adapted from
    // linux's i8259.c.
    outb(ICW1, MASTER_8259_CMD);
    outb(ICW1, SLAVE_8259_CMD);
    outb(ICW2_MASTER, MASTER_8259_DATA);    // Master PIC and Slave PIC
    outb(ICW2_SLAVE, SLAVE_8259_DATA);      // are initialized differently.
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

    outb(master_mask, MASTER_8259_DATA);    // Initially, mask all interrupts,
    outb(slave_mask, SLAVE_8259_DATA);      // only enable when needed

    // Enable slave 8259's interrupts to be processed on master 8259.
    enable_irq(SLAVE_8259_PORT);
}

/* void enable_irq(uint32_t irq_num)
 * @input: irq_num - the IRQ to be enabled (unmasked). Range 0-15.
 * @effects: corresponding bit of IRQ is set to 0,
 *           mask is transferred to PIC
 * @description: enable an IRQ line on PIC.
 */
void enable_irq(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return; // Only accept range 0-15.
    uint8_t change_mask = 0;
    if(irq_num >= I8259_PORT_COUNT) {
        change_mask = 1 << (irq_num - I8259_PORT_COUNT);    // Put 1 on th (irq_num)th bit.
        slave_mask &= ~change_mask; // Flip mask so all bits but (irq_num)th is 1,
                                    // so we can remove the (irq_num)th bit on mask.
        outb(slave_mask, SLAVE_8259_DATA);  // Write mask into 8259.
    } else {
        change_mask = 1 << irq_num; // Put 1 on th (irq_num)th bit.
        master_mask &= ~change_mask;// Flip mask so all bits but (irq_num)th is 1,
                                    // so we can remove the (irq_num)th bit on mask.
        outb(master_mask, MASTER_8259_DATA);// Write mask into 8259.
    }
}

/* void disable_irq(uint32_t irq_num)
 * @input: irq_num - the IRQ to be disabled (masked). Range 0-15.
 * @effects: corresponding bit of IRQ is set to 1,
 *           mask is transferred to PIC
 * @description: disable an IRQ line on PIC.
 */
void disable_irq(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return; // Only accept range 0-15.
    uint8_t change_mask = 0;
    if(irq_num >= I8259_PORT_COUNT) {
        change_mask = 1 << (irq_num - I8259_PORT_COUNT);    // Put 1 on th (irq_num)th bit.
        slave_mask |= change_mask;          // Set the (irq_num)th bit of the mask.
        outb(slave_mask, SLAVE_8259_DATA);  // Write mask into 8259.
    } else {
        change_mask = 1 << irq_num;         // Put 1 on th (irq_num)th bit.
        master_mask |= change_mask;         // Set the (irq_num)th bit of the mask.
        outb(master_mask, MASTER_8259_DATA);// Write mask into 8259.
    }
}

/* void send_eoi(uint32_t irq_num)
 * @input: irq_num - the IRQ whose EOI is to be sent.
 * @effects: EOI is sent to PIC.
 *           If the IRQ resides on slave 8259, EOI is also sent for slave 8259's
 *           line on master 8259.
 * @description: Send EOI to 8259, prepare it for handling next interrupt.
 */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 2 * I8259_PORT_COUNT) return; // Only accept range 0-15.
    uint8_t data = EOI;
    if(irq_num >= I8259_PORT_COUNT) {
        data |= irq_num - I8259_PORT_COUNT; // OR the data with IRQ num, as specs require.
        outb(data, SLAVE_8259_CMD);         // send the EOI to slave 8259.
        send_eoi(SLAVE_8259_PORT);          // Also send EOI for slave 8259's line on master 8259.
    } else {
        data |= irq_num;                    // OR the data with IRQ num, as specs require.
        outb(data, MASTER_8259_CMD);        // send the EOI to master 8259.
    }
}
