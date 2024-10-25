#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t idx;
    uintptr_t next;
    uintptr_t data_addr;
} vec_node_t;

typedef struct {
    uint64_t data_size;
    size_t len;
    vec_node_t *first_node;
} vec_t;

vec_t *vec_new(uint64_t elem_size);
void vec_push(vec_t *vec, uintptr_t addr);
void *vec_at(vec_t *vec, size_t idx);
void vec_pop(vec_t *vec, size_t idx);
void vec_set(vec_t *vec, size_t idx, uintptr_t val);
void vec_free(vec_t *vec);
size_t vec_size(vec_t *vec);
