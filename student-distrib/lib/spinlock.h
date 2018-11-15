#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "lib.h"

typedef struct {
    uint32_t lock;
} spinlock_t;

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED 1

void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);
uint32_t spin_trylock(spinlock_t* lock);

#endif
