#include "pit.h"
#include "i8259.h"
#include "../interrupts/multiprocessing.h"

void pit_init() {
    outb(PIT_MODE, PIT_REG_CMD);
    outb((uint8_t) PIT_INTERVAL, PIT_REG_DATA);
    outb((uint8_t) (PIT_INTERVAL >> 8), PIT_REG_DATA);

    enable_irq(PIT_IRQ);
}

void pit_interrupt() {
    send_eoi(PIT_IRQ);
    int32_t new_terminal = (active_terminal_id + 1) % TERMINAL_COUNT;
    // while(-1 == terminals[new_terminal].active_process) {
    //     new_terminal = (new_terminal + 1) % TERMINAL_COUNT;
    // }
    terminal_switch_active(new_terminal);
}
