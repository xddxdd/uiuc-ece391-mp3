#ifndef _SB16_H_
#define _SB16_H_

#include "../lib/lib.h"

#define SB16_BUF_ADDR 0x10000
#define SB16_BUF_MID 0x18000
#define SB16_BUF_LEN 0xffff
#define SB16_BUF_LEN_HALF 0x7fff

#define SB16_CALL_SUCCESS 0
#define SB16_CALL_FAIL -1

#define SB16_IRQ 5

#define SB16_PORT_RESET 0x226
#define SB16_PORT_READ 0x22a
#define SB16_PORT_WRITE 0x22c
#define SB16_PORT_STATUS 0x22e

#define SB16_STATUS_READY 0xaa

// Only dealing with 8 bit depth music playing
#define SB16_CMD_SAMPLING_RATE 0x41
#define SB16_CMD_PAUSE 0xd0
#define SB16_CMD_CONTINUE 0xd4
#define SB16_CMD_EXIT_AFTER_BLOCK 0xda
#define SB16_CMD_PLAY 0xc6

#define SB16_MODE_MONO 0x00
#define SB16_MODE_STEREO 0x20
#define SB16_MODE_SIGNED 0x10
#define SB16_MODE_UNSIGNED 0x00

#define DMA_MASK_CHANNEL 0x04
#define DMA_UNMASK_CHANNEL 0x00
#define DMA_SELECT_CHANNEL_1 0x01
#define DMA_FLIPFLOP_RESET 0xff

#define DMA_REG_CH1_START 0x02
#define DMA_REG_CH1_SIZE 0x03
#define DMA_REG_CH1_PAGE 0x83
#define DMA_REG_FLIPFLOP 0x0c
#define DMA_REG_CHANNEL_MASK 0x0a
#define DMA_REG_MODE 0x0b

#define DMA_MODE_SELFTEST 0x00
#define DMA_MODE_PERIPH_WRITE 0x04
#define DMA_MODE_PERIPH_READ 0x08
#define DMA_MODE_AUTO 0x10
#define DMA_MODE_DOWN 0x20
#define DMA_MODE_TRANSFER_ONDEMAND 0x00
#define DMA_MODE_TRANSFER_SINGLE 0x40
#define DMA_MODE_TRANSFER_BLOCK 0x80
#define DMA_MODE_TRANSFER_CASCADE 0xc0

int32_t sb16_init();
int32_t sb16_play(uint16_t sampling_rate, uint8_t is_stereo, uint8_t is_signed);
int32_t sb16_continue();
int32_t sb16_pause();
int32_t sb16_stop_after_block();
int32_t sb16_read();
void sb16_interrupt();

#endif
