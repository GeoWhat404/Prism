#pragma once

#include <mem/heap.h>

#define malloc(ptr) kmalloc(ptr)
#define free(ptr) kfree(ptr)

void abort();
