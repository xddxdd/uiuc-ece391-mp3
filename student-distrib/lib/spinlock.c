#include "spinlock.h"

/* static inline unsigned xchgl(void *ptr, unsigned x)
 * @input: ptr - pointer containing data to be exchanged
 *         x - value to be exchanged
 * @output: the two values are exchanged
 * @description: XCHGL instruction with LOCK, for future multi process cooperation
 */
static inline unsigned xchgl(void *ptr, unsigned x) {
	__asm__ __volatile__(
        "lock xchgl %0,%1"
		: "=r" ((unsigned) x)
		: "m" (*(volatile unsigned *)ptr), "0" (x)
		: "memory");
	return x;
}

/* void spin_lock(spinlock_t* lock)
 * @input: lock - spinlock we're trying to lock
 * @output: the lock is locked by current process
 * @description: try indefinitely until the lock is locked
 */
void spin_lock(spinlock_t* lock) {
    while(SPINLOCK_LOCKED == xchgl(&lock->lock, SPINLOCK_LOCKED)) {
        // TODO: tell scheduler put thread to sleep
    }
}

/* void spin_unlock(spinlock_t* lock)
 * @input: lock - spinlock we're trying to unlock
 * @output: the lock is unlocked
 * @description: unlock the lock
 */
void spin_unlock(spinlock_t* lock) {
    lock->lock = SPINLOCK_UNLOCKED;
}

/* uint32_t spin_trylock(spinlock_t* lock)
 * @input: lock - spinlock we're trying to lock
 * @output: whether the lock is locked by us
 * @description: try once to lock the lock
 */
uint32_t spin_trylock(spinlock_t* lock) {
    return !xchgl(&lock->lock, SPINLOCK_LOCKED);
}
