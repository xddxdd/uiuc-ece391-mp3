#include "sb16.h"
#include "i8259.h"

volatile uint8_t sb16_used = 0;          // Whether SB16 is being used exclusively
volatile uint8_t sb16_interrupted = 0;   // Interrupt counter, used for sb16_read()

/* int32_t sb16_init()
 * @output: Sound Blaster 16 initialized
 *          ret val - SUCCESS / FAIL
 * @description: Initialize Sound Blaster 16 sound card.
 */
int32_t sb16_init() {
    // Sound Blaster 16 initialization sequence,
    // as described in the code in https://wiki.osdev.org/Sound_Blaster_16
    cli();
    if(sb16_used) {
        sti();
        return FAIL;
    }
    outb(1, SB16_PORT_RESET);   // Enter reset mode
    int i = 0;                  // Wait 10000 cycles,
    for(i = 0; i < 10000; i++); // around 3us assuming 3GHz CPU
    outb(0, SB16_PORT_RESET);   // Leave reset mode
    // Verify if SB16 is present
    uint8_t data = inb(SB16_PORT_READ);

    if(data != SB16_STATUS_READY) {     // If SB16 isn't present, quit
        sti();
        return FAIL;
    }
    sb16_used = 1;

    enable_irq(SB16_IRQ);   // Enable its IRQ for music transmission

    // DMA initialization
    // 1. Mask DMA channel 1
    outb(DMA_MASK_CHANNEL | DMA_SELECT_CHANNEL_1, DMA_REG_CHANNEL_MASK);
    // 2. Reset flip-flop register
    outb(DMA_FLIPFLOP_RESET, DMA_REG_FLIPFLOP);
    // 3. Send start address of DMA, bit 0-7 then bit 8-15
    outb((uint8_t) (SB16_BUF_ADDR), DMA_REG_CH1_START);
    outb((uint8_t) (SB16_BUF_ADDR >> 8), DMA_REG_CH1_START);
    // 4. Reset flip-flop register
    outb(DMA_FLIPFLOP_RESET, DMA_REG_FLIPFLOP);
    // 5. Send size of DMA, bit 0-7 then bit 8-15
    outb((uint8_t) (SB16_BUF_LEN - 1), DMA_REG_CH1_SIZE);
    outb((uint8_t) ((SB16_BUF_LEN - 1) >> 8), DMA_REG_CH1_SIZE);
    // 6. Send page id of DMA, bit 16-24 of buffer address
    outb((uint8_t) (SB16_BUF_ADDR >> 16), DMA_REG_CH1_PAGE);
    // 7. Set DMA mode to single transfer, peripheral reading, auto loop
    outb(DMA_SELECT_CHANNEL_1 | DMA_MODE_PERIPH_READ
        | DMA_MODE_AUTO | DMA_MODE_TRANSFER_SINGLE, DMA_REG_MODE);
    // 8. Unmask DMA channel 1
    outb(DMA_UNMASK_CHANNEL | DMA_SELECT_CHANNEL_1, DMA_REG_CHANNEL_MASK);
    sti();

    return SUCCESS;
}

// Unified FS interface.
unified_fs_interface_t sb16_if = {
    .open = sb16_open,
    .read = sb16_read,
    .write = sb16_write,
    .ioctl = sb16_ioctl,
    .close = sb16_close
};

/* int32_t sb16_open(int32_t* inode, char* filename)
 * @input: all ignored
 * @output: sound blaster 16 initialized and locked for exclusive use
 *          ret val - SUCCESS / FAIL
 * @description: initializes sound blaster 16 for audio output.
 */
int32_t sb16_open(int32_t* inode, char* filename) {
    return sb16_init();
}

/* int32_t sb16_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len)
 * @output: waits after next SB16 interrupt occurs.
 *          ret val - SUCCESS / FAIL
 * @description: wait until next SB16 interrupt, so we can copy
 *     the next block of music into buffer.
 */
int32_t sb16_read(int32_t* inode, uint32_t* offset, char* buf, uint32_t len) {
    if(!sb16_used) return FAIL;   // If SB16 isn't present, quit
    uint8_t prev_id = sb16_interrupted;
    while(prev_id == sb16_interrupted) wait_interrupt();    // Wait until the interrupt state changed
    return SUCCESS;
}

/* int32_t sb16_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len)
 * @input: offset - current pos in SB16 buffer
 *         buf - data to be copied into
 *         len - length of buf
 * @output: SB16 buffer written with data from buf
 *          ret val - SUCCESS / FAIL
 * @description: copy audio segment into sound blaster 16's buffer.
 */
int32_t sb16_write(int32_t* inode, uint32_t* offset, const char* buf, uint32_t len) {
    if(!sb16_used) return FAIL;   // If SB16 isn't present, quit
    if(NULL == buf) return FAIL;

    if(*offset + len <= SB16_BUF_LEN) {
        memcpy((char*) (SB16_BUF_ADDR + *offset), buf, len);
        (*offset) += len;
    } else {
        uint32_t len_to_copy = SB16_BUF_LEN - *offset;
        memcpy((char*) (SB16_BUF_ADDR + *offset), buf, len_to_copy);
        *offset = 0;
        if(len - len_to_copy != sb16_write(inode, offset, buf + len_to_copy, len - len_to_copy)) return FAIL;
    }
    return len;
}

/* int32_t sb16_ioctl(int32_t* inode, uint32_t* offset, int32_t op)
 * @input: op - command to be sent to sound blaster 16
 * @output: command sent to sound blaster 16
 *          ret val - SUCCESS / FAIL
 * @description: sends command to control sound blaster 16.
 *     used for pausing, resuming, setting params, etc.
 */
int32_t sb16_ioctl(int32_t* inode, uint32_t* offset, int32_t op) {
    if(!sb16_used) return FAIL;   // If SB16 isn't present, quit
    outb((uint8_t) op, SB16_PORT_WRITE);
    return SUCCESS;
}

/* int32_t sb16_close(int32_t* inode)
 * @input: all ignored
 * @output: sound blaster 16 lock released for other program's use
 *          ret val - SUCCESS / FAIL
 * @description: releases lock on sound blaster 16.
 */
int32_t sb16_close(int32_t* inode) {
    if(!sb16_used) return FAIL;   // If SB16 isn't present, quit
    // outb(SB16_CMD_PAUSE, SB16_PORT_WRITE);
    sb16_used = 0;
    return SUCCESS;
}

/* sb16_interrupt()
 * @description: Interrupt handler of SB16.
 *     Updates interrupt state variable, queries SB16 status port,
 *     and sends EOI.
 */
void sb16_interrupt() {
    sb16_interrupted++;
    inb(SB16_PORT_STATUS);
    send_eoi(SB16_IRQ);
}
