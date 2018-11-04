#include "sb16.h"
#include "i8259.h"

uint8_t sb16_here = 0;          // Store whether SB16 has been initialized
volatile uint8_t sb16_interrupted = 0;   // Interrupt counter, used for sb16_read()

/* int32_t sb16_init()
 * @output: Sound Blaster 16 initialized
 *          ret val - SUCCESS / FAIL
 * @description: Initialize Sound Blaster 16 sound card.
 */
int32_t sb16_init() {
    if(sb16_here) return SB16_CALL_SUCCESS; // Don't initialize again

    // Sound Blaster 16 initialization sequence,
    // as described in the code in https://wiki.osdev.org/Sound_Blaster_16
    cli();
    outb(1, SB16_PORT_RESET);   // Enter reset mode
    int i = 0;                  // Wait 10000 cycles,
    for(i = 0; i < 10000; i++); // around 3us assuming 3GHz CPU
    outb(0, SB16_PORT_RESET);   // Leave reset mode
    // Verify if SB16 is present
    uint8_t data = inb(SB16_PORT_READ);
    sb16_here = data == SB16_STATUS_READY;
    sti();

    if(!sb16_here) return SB16_CALL_FAIL;   // If SB16 isn't present, quit

    printf("Sound Blaster 16 Detected\n");
    cli();
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
    outb((uint8_t) (SB16_BUF_LEN), DMA_REG_CH1_SIZE);
    outb((uint8_t) (SB16_BUF_LEN >> 8), DMA_REG_CH1_SIZE);
    // 6. Send page id of DMA, bit 16-24 of buffer address
    outb((uint8_t) (SB16_BUF_ADDR >> 16), DMA_REG_CH1_PAGE);
    // 7. Set DMA mode to single transfer, peripheral reading, auto loop
    outb(DMA_SELECT_CHANNEL_1 | DMA_MODE_PERIPH_READ
        | DMA_MODE_AUTO | DMA_MODE_TRANSFER_SINGLE, DMA_REG_MODE);
    // 8. Unmask DMA channel 1
    outb(DMA_UNMASK_CHANNEL | DMA_SELECT_CHANNEL_1, DMA_REG_CHANNEL_MASK);
    sti();

    return SB16_CALL_SUCCESS;
}

/* int32_t sb16_play(uint16_t sampling_rate. uint8_t stereo, uint8_t signed)
 * @input: sampling_rate - sampling rate to be set, max 44100.
 *         is_stereo - whether music is stereo, 0 or 1.
 *         is_signed - whether PCM data is signed, 0 or 1.
 * @output: SB16 starts to play music in buffer
 *          ret val - SUCCESS / FAIL
 * @description: Set parameters for music playback, and starts playing.
 */
int32_t sb16_play(uint16_t sampling_rate, uint8_t is_stereo, uint8_t is_signed) {
    if(!sb16_here) return SB16_CALL_FAIL;   // Don't do anything if device not present
    if(sampling_rate > 44100) return SB16_CALL_FAIL;    // Sampling rate too high
    // 1. Send command of changing sampling rate
    outb(SB16_CMD_SAMPLING_RATE, SB16_PORT_WRITE);
    // 2. Send the sampling rate, high 8 bit first, then low 8 bit
    outb((uint8_t) (sampling_rate >> 8), SB16_PORT_WRITE);
    outb((uint8_t) sampling_rate, SB16_PORT_WRITE);
    // 3. Send command for prepare to play
    outb(SB16_CMD_PLAY, SB16_PORT_WRITE);
    // 4. Send mode of music, stereo/mono and signed/unsigned
    outb((is_stereo > 0 ? SB16_MODE_STEREO : SB16_MODE_MONO)
        | (is_signed > 0 ? SB16_MODE_SIGNED : SB16_MODE_UNSIGNED),
        SB16_PORT_WRITE);
    // 5. Send block size, note that block size is half of buffer length
    //    so we can utilize double buffering and copy the next block of music
    //    when one block runs out, to ensure smooth playing
    outb((uint8_t) SB16_BUF_LEN_HALF, SB16_PORT_WRITE);
    outb((uint8_t) (SB16_BUF_LEN_HALF >> 8), SB16_PORT_WRITE);

    return SB16_CALL_SUCCESS;
}

/* int32_t sb16_continue()
 * @output: SB16 continues to play music in buffer
 *          ret val - SUCCESS / FAIL
 * @description: Continues to play music in buffer.
 */
int32_t sb16_continue() {
    if(!sb16_here) return SB16_CALL_FAIL;   // If SB16 isn't present, quit
    outb(SB16_CMD_CONTINUE, SB16_PORT_WRITE);
    return SB16_CALL_SUCCESS;
}

/* int32_t sb16_pause()
 * @output: SB16 pauses playing music in buffer
 *          ret val - SUCCESS / FAIL
 * @description: Pauses playing music in buffer.
 */
int32_t sb16_pause() {
    if(!sb16_here) return SB16_CALL_FAIL;   // If SB16 isn't present, quit
    outb(SB16_CMD_PAUSE, SB16_PORT_WRITE);
    return SB16_CALL_SUCCESS;
}

/* int32_t sb16_stop_after_block()
 * @output: SB16 stops playing music after this block.
 *          ret val - SUCCESS / FAIL
 * @description: Stop playing music after this block.
 *     useful when music is finished.
 */
int32_t sb16_stop_after_block() {
    if(!sb16_here) return SB16_CALL_FAIL;   // If SB16 isn't present, quit
    outb(SB16_CMD_EXIT_AFTER_BLOCK, SB16_PORT_WRITE);
    return SB16_CALL_SUCCESS;
}

/* int32_t sb16_read()
 * @output: function waits after next SB16 interrupt occurs.
 *          ret val - SUCCESS / FAIL
 * @description: wait until next SB16 interrupt, so we can copy
 *     the next block of music into buffer.
 */
int32_t sb16_read() {
    if(!sb16_here) return SB16_CALL_FAIL;   // If SB16 isn't present, quit
    uint8_t prev_id = sb16_interrupted;
    while(prev_id == sb16_interrupted); // Wait until the interrupt state changed
    return SB16_CALL_SUCCESS;
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
