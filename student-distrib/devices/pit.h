#ifndef _PIT_H_
#define _PIT_H_

#include "../lib/lib.h"

#define PIT_IRQ         0
#define PIT_REG_CMD     0x43
#define PIT_REG_DATA    0x40
#define PIT_MODE        0x37    // Channel 0, Two byte, Mode 3
#define PIT_INTERVAL    11932   // 100Hz as PIT is 1.19318 MHz

void pit_init();
void pit_interrupt();

#endif
