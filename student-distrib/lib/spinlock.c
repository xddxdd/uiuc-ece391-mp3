#include "spinlock.h"

static inline unsigned xchgl(void *ptr, unsigned x) {
	__asm__ __volatile__(
        "lock xchgl %0,%1"
		: "=r" ((unsigned) x)
		: "m" (*(volatile unsigned *)ptr), "0" (x)
		: "memory");
	return x;
}

void spin_lock(spinlock_t* lock) {
    while(SPINLOCK_LOCKED == xchgl(&lock->lock, SPINLOCK_LOCKED)) {
        // TODO: tell scheduler put thread to sleep
    }
}

void spin_unlock(spinlock_t* lock) {
    lock->lock = SPINLOCK_UNLOCKED;
}

uint32_t spin_trylock(spinlock_t* lock) {
    return !xchgl(&lock->lock, SPINLOCK_LOCKED);
}
