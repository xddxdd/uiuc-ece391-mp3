#ifndef _SB16_H_
#define _SB16_H_

#include "../lib.h"

#define SB16_BUF_LEN 0x10000

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

#endif
