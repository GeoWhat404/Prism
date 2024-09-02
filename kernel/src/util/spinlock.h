#pragma once

#include <stdatomic.h>
#include <stdint.h>

typedef atomic_flag spinlock_t;

typedef struct {
    int64_t count;
} spinlock_count_t;

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
void spinlock_wait(spinlock_t *lock);

void spinlock_count_read_acquire(spinlock_count_t *lock);
void spinlock_count_read_release(spinlock_count_t *lock);

void spinlock_count_write_acquire(spinlock_count_t *lock);
void spinlock_count_write_release(spinlock_count_t *lock);
