#include "serial.h"
#include "i8259.h"

#define SERIAL_PORTS_COUNT 2
uint16_t serial_ports[] = {0x3f8, 0x2f8};
uint8_t serial_irqs[] = {4, 3};

int8_t serial_init(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    outb(0x00, port + 1);   // Disable interrupts
    outb(0x80, port + 3);   // Enable DLAB to set baud rate
    // We use baud rate 9600 with 8/N/1 format
    outb(0x0c, port + 0);   // Baud rate divisor 115200 / 9600 = 12, low byte
    outb(0x00, port + 1);   // Baud rate divisor, high byte
    outb(0x03, port + 3);   // Disable DLAB, set format to 8/N/1
    outb(0xc7, port + 2);   // Enable FIFO
    outb(0x0f, port + 4);   // Enable interrupt only for data available
    enable_irq(serial_irqs[id]);
    return SERIAL_OP_SUCCESS;
}

int8_t serial_is_available_rx(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    return inb(port + 5) & 0x01;
}

int8_t serial_is_available_tx(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    return inb(port + 5) & 0x20;
}

int8_t serial_read(uint8_t id) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    while(!serial_is_available_rx(id));
    return inb(port);
}

int8_t serial_write(uint8_t id, uint8_t data) {
    if(id >= SERIAL_PORTS_COUNT) return SERIAL_OP_FAIL;
    uint16_t port = serial_ports[id];
    while(!serial_is_available_tx(id));
    outb(data, port);
    return SERIAL_OP_SUCCESS;
}

void serial_interrupt(uint8_t id) {
    printf("IRQ%d", id);
    while(serial_is_available_rx(id)) {
        printf("COM%d: %x\n", id + 1, serial_read(id));
        /*switch(i) {
            case 0:
                printf("Serial message");
            default: break;
        }*/
    }
}

void serial1_interrupt() { serial_interrupt(COM1); }
void serial2_interrupt() { serial_interrupt(COM2); }
