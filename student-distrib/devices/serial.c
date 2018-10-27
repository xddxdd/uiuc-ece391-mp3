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
int8_t serial_init(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    outb(0x00, port + 1);   // Disable interrupts
    outb(0x80, port + 3);   // Enable DLAB to set baud rate
    // We use baud rate 9600 with 8/N/1 format
    outb(0x0c, port + 0);   // Baud rate divisor 115200 / 9600 = 12, low byte
    outb(0x00, port + 1);   // Baud rate divisor, high byte
    outb(0x03, port + 3);   // Disable DLAB, set format to 8/N/1
    outb(0x01, port + 1);   // Enable interrupt only for data available
    outb(0xc7, port + 2);   // Enable FIFO
    outb(0x0b, port + 4);   // Enable FIFO
    enable_irq(serial_irqs[id]);
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
    return inb(port + 5) & 0x01;
}

/* int8_t serial_is_available_tx(uint8_t id)
 * @input: id - ID of com port to be checked
 * @output: 0 if data should not be sent, 1 otherwise
 * @description: checks if the port is ready for sending data.
 */
int8_t serial_is_available_tx(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    return inb(port + 5) & 0x20;
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
    return inb(port);
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
    outb(data, port);
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
