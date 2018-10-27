#ifndef _SB16_H_
#define _SB16_H_

#include "../lib.h"

#define SB16_BUF_ADDR 0x10000
#define SB16_BUF_MID 0x18000
#define SB16_BUF_LEN 0xffff
#define SB16_BUF_LEN_HALF 0x7fff

#define SB16_IRQ 5

#define SB16_PORT_RESET 0x226
#define SB16_PORT_READ 0x22a
#define SB16_PORT_WRITE 0x22c
#define SB16_PORT_STATUS 0x22e

// Only dealing with 8 bit depth music playing
#define SB16_CMD_SAMPLING_RATE 0x41
#define SB16_CMD_PAUSE 0xd0
#define SB16_CMD_CONTINUE 0xd4
#define SB16_CMD_EXIT_AFTER_BLOCK 0xda
#define SB16_CMD_INIT 0xc6

void sb16_init();
void sb16_set_sampling_rate(uint16_t sampling_rate);
void sb16_write(char* buf, uint16_t len);
void sb16_play();
void sb16_continue();
void sb16_pause();
void sb16_stop_after_block();
void sb16_read();
void sb16_interrupt();

#endif
