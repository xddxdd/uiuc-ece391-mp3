#include "pit.h"
#include "i8259.h"
#include "../interrupts/multiprocessing.h"

// Counter to maintain system time
volatile uint32_t pit_timer = 0;

/* void pit_init()
 * @output: PIT generates interrupts at interval specified by PIT_INTERVAL
 * @description: initializes PIT for scheduling.
 */
void pit_init() {
    outb(PIT_MODE, PIT_REG_CMD);
    outb((uint8_t) PIT_INTERVAL, PIT_REG_DATA);
    outb((uint8_t) (PIT_INTERVAL >> 8), PIT_REG_DATA);

    enable_irq(PIT_IRQ);
}

/* void pit_interrupt()
 * @output: system switch to another process, for multiprocessing.
 * @description: switches between processes to achieve background multiprocessing.
 */
void pit_interrupt() {
    // Increment system time counter
    pit_timer++;
    send_eoi(PIT_IRQ);
    // Do a context switch
    int32_t new_terminal = (active_terminal_id + 1) % TERMINAL_COUNT;
    terminal_switch_active(new_terminal);
}

/* void pit_sleep(uint32_t ms)
 * @input: ms - milliseconds to wait
 * @output: program waits for that many time
 * @description: the sleep() function.
 */
void pit_sleep(uint32_t ms) {
    uint32_t curr_time = pit_timer;
    uint32_t ticks = ms / (MS_IN_S / PIT_FREQ);
    uint32_t target_time = curr_time + ticks;
    if(target_time < curr_time) {
        // Timer wrap-around will happen, first wait for timer to wrap
        while(pit_timer >= curr_time) wait_interrupt();
    }
    while(pit_timer < target_time) wait_interrupt();
}
