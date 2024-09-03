#include "spinlock.h"

#include <hal/panic.h>
#include <util/debug.h>

void spinlock_acquire(spinlock_t *lock) {
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        __asm__ volatile("pause");
    }
}

void spinlock_release(spinlock_t *lock) {
    atomic_flag_clear_explicit(lock, memory_order_release);
}

void spinlock_wait(spinlock_t *lock) {
    while (atomic_flag_test_and_set_explicit(lock, memory_order_relaxed)) {
        atomic_flag_clear_explicit(lock, memory_order_relaxed);
    }
}

void spinlock_count_read_acquire(spinlock_count_t *lock) {
    while (lock->count < 0)
        __asm__ volatile("pause");
    lock->count++;
}

void spinlock_count_read_release(spinlock_count_t *lock) {
    if (lock->count < 0) {
        log_error(MODULE_SPINLOCK, "Expected a positive count value");
        panic("spinlock: positive value expected");
    }
    lock->count--;
}

void spinlock_count_write_acquire(spinlock_count_t *lock) {
    while (lock->count != 0)
        __asm__ volatile("pause");
    lock->count = -1;
}

void spinlock_count_write_release(spinlock_count_t *lock) {
    if (lock->count != -1) {
        log_error(MODULE_SPINLOCK, "Expected -1 but got %lld", lock->count);
        panic("spinlock: expected count to be -1 (%lld)", lock->count);
    }
    lock->count = 0;
}
