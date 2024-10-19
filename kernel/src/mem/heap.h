#pragma once

#include <stddef.h>
#include <stdint.h>

void heap_init(void *heap_addr, size_t size);
void heap_print(void);

void *kmalloc(size_t size);
void kfree(void *ptr);
