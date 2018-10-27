#include "sb16.h"
#include "i8259.h"

uint8_t sb16_here = 0;
uint8_t sb16_interrupted = 0;

void sb16_init() {
    if(sb16_here) return; // Don't initialize again

    outb(1, SB16_PORT_RESET);
    int i = 0; for(i = 0; i < 10000; i++);  // Wait 10000 cycles,
                                            // around 3us assuming 3GHz CPU
    outb(0, SB16_PORT_RESET);
    uint8_t data = inb(SB16_PORT_READ);
    sb16_here = data == 0xaa;

    if(!sb16_here) return;

    printf("Sound Blaster 16 Detected\n");
    sb16_set_sampling_rate(22050);
    enable_irq(SB16_IRQ);

    // DMA initialization
    outb(0x05, 0x0a);   // mask DMA channel 1
    outb(0xff, 0x0c);   // reset flip-flop
    outb(0x00, 0x02);   // send start addr, rightmost byte
    outb(0x00, 0x02);   // send start addr, next byte
    outb(0xff, 0x03);   // send size, rightmost byte
    outb(0xff, 0x03);   // send size, leftmost byte
    outb(0x01, 0x83);   // send page addr, leftmost byte of start addr
    outb(0x59, 0x0b);   // DMA mode, single transfer, auto loop
    outb(0x01, 0x0a);   // unmask DMA channel 1
}

void sb16_set_sampling_rate(uint16_t sampling_rate) {
    if(!sb16_here) return;
    outb(SB16_CMD_SAMPLING_RATE, SB16_PORT_WRITE);
    outb((sampling_rate >> 8) & 0xff, SB16_PORT_WRITE);
    outb(sampling_rate & 0xff, SB16_PORT_WRITE);
}

void sb16_write(char* buf, uint16_t len) {
    char* sb16_dma = (char*) SB16_BUF_ADDR;
    memcpy(sb16_dma, buf, len);
}

void sb16_play() {
    printf("Play\n");
    // Set output mode
    outb(SB16_CMD_INIT, SB16_PORT_WRITE);
    outb(0x00, SB16_PORT_WRITE);
    outb(0xff, SB16_PORT_WRITE);
    outb(0x7f, SB16_PORT_WRITE);
}

void sb16_continue() {
    outb(SB16_CMD_CONTINUE, SB16_PORT_WRITE);
}

void sb16_pause() {
    outb(SB16_CMD_PAUSE, SB16_PORT_WRITE);
}

void sb16_stop_after_block() {
    outb(SB16_CMD_EXIT_AFTER_BLOCK, SB16_PORT_WRITE);
}

void sb16_read() {
    uint8_t prev_id = sb16_interrupted;
    while(prev_id == sb16_interrupted);
}

void sb16_interrupt() {
    sb16_interrupted++;
    printf("I");
    inb(SB16_PORT_STATUS);
    send_eoi(SB16_IRQ);
}
