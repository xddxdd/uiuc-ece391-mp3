#include "serial.h"
#include "i8259.h"
#include "tux.h"

// Serial port definitions, corresponding to values in QEMU source code.
// May or may not work on actual machines.
#define SERIAL_PORTS_COUNT 2
uint16_t serial_ports[] = {0x3f8, 0x2f8};
uint8_t serial_irqs[] = {4, 3};

/* int8_t serial_init(uint8_t id)
 * @input: id - ID of com port to be initialized
 * @output: the port gets initialized to following mode:
 *          - Interrupt enabled for receiving data
 *          - 9600 baudrate, 8/N/1
 *          ret val: SUCCESS / FAIL
 * @description: initializes serial port.
 */
int8_t serial_init(uint8_t id, uint32_t baud_rate) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    cli();
    uint16_t port = serial_ports[id];
    // Disable interrupts
    outb(SERIAL_INTERRUPT_DISABLE, port + SERIAL_REG_OFFSET_INTERRUPT_ENABLE);
    // Enable DLAB to set baud rate
    outb(SERIAL_LINE_CONTROL_DLAB, port + SERIAL_REG_OFFSET_LINE_CONTROL);
    // We use baud rate 9600 with 8/N/1 format, so the divisor is 12
    uint32_t divisor = SERIAL_BASE_BAUDRATE / baud_rate;
    outb((uint8_t) divisor, port + SERIAL_REG_OFFSET_BAUDRATE_LOWBYTE);
    outb((uint8_t) (divisor >> 8), port + SERIAL_REG_OFFSET_BAUDRATE_HIGHBYTE);
    // Disable DLAB, set format to 8/N/1
    outb(SERIAL_LINE_CONTROL_8N1, port + SERIAL_REG_OFFSET_LINE_CONTROL);
    // Enable interrupt only for data available
    outb(SERIAL_INTERRUPT_ENABLE_DATA, port + SERIAL_REG_OFFSET_INTERRUPT_ENABLE);
    // Enable serial modem FIFO
    outb(SERIAL_INTERRUPT_ID, port + SERIAL_REG_OFFSET_INTERRUPT_IDENTIFICATION);
    outb(SERIAL_MODEM_CONTROL, port + SERIAL_REG_OFFSET_MODEM_CONTROL);
    // Enable IRQ and done
    enable_irq(serial_irqs[id]);
    sti();
    return SERIAL_OP_SUCCESS;
}

/* int8_t serial_is_available_rx(uint8_t id)
 * @input: id - ID of com port to be checked
 * @output: 0 if no data is available, 1 otherwise
 * @description: checks if the port has something in its receiving buffer.
 */
int8_t serial_is_available_rx(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    // The state is in bit 0 of line status register, hence the magic number
    return inb(port + SERIAL_REG_OFFSET_LINE_STATUS) & 0x01;
}

/* int8_t serial_is_available_tx(uint8_t id)
 * @input: id - ID of com port to be checked
 * @output: 0 if data should not be sent, 1 otherwise
 * @description: checks if the port is ready for sending data.
 */
int8_t serial_is_available_tx(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    // The state is in bit 5 of line status register, hence the magic number
    return inb(port + SERIAL_REG_OFFSET_LINE_STATUS) & 0x20;
}

/* int8_t serial_read(uint8_t id)
 * @input: id - ID of com port to be checked
 * @output: -1 if no data is available,
 *          the data otherwise
 * @description: read a byte from serial port.
 */
int8_t serial_read(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    while(!serial_is_available_rx(id));
    return inb(port + SERIAL_REG_OFFSET_DATA);
}

/* int8_t serial_write(uint8_t id, uint8_t data)
 * @input: id - ID of com port to be checked
 *         data - data to be written
 * @output: SUCCESS / FAIL
 * @description: write a byte to serial port.
 */
int8_t serial_write(uint8_t id, uint8_t data) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    while(!serial_is_available_tx(id));
    outb(data, port + SERIAL_REG_OFFSET_DATA);
    return SERIAL_OP_SUCCESS;
}

/* void serial_interrupt(uint8_t id)
 * @input: id - ID of com port to be checked
 * @description: interrupt handler of serial events
 */
void serial_interrupt(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return;
    while(serial_is_available_rx(id)) {
        char data = serial_read(id);
        if(id == TC_SERIAL_PORT) tux_interrupt(data);
    }
    send_eoi(serial_irqs[id]);
}

/* void serial1_interrupt() - interrupt handler wrapper for previous function */
void serial1_interrupt() { serial_interrupt(COM1); }
/* void serial2_interrupt() - interrupt handler wrapper for previous function */
void serial2_interrupt() { serial_interrupt(COM2); }
