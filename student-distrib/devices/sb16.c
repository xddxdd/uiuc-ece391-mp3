#include "sb16.h"

char buf[SB16_BUF_LEN];

uint8_t sb16_here = 0;

void sb16_init() {
    if(sb16_here) return; // Don't initialize again

    outb(1, SB16_PORT_RESET);
    int i = 0; for(i = 0; i < 10000; i++);  // Wait 10000 cycles,
                                            // around 3us assuming 3GHz CPU
    outb(0, SB16_PORT_RESET);
    uint8_t data = inb(SB16_PORT_READ);
    sb16_here = data == 0xaa;

    sb16_set_sampling_rate(44100);
}

void sb16_set_sampling_rate(uint16_t sampling_rate) {
    if(!sb16_here) return;
    outb(SB16_CMD_SAMPLING_RATE, SB16_PORT_WRITE);
    outb((sampling_rate >> 8) & 0xff, SB16_PORT_WRITE);
    outb(sampling_rate & 0xff, SB16_PORT_WRITE);
}
