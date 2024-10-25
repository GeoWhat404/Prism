#include "vec.h"
#include <mem/heap.h>

vec_t *vec_new(uint64_t elem_size) {
    vec_t *vec = kmalloc(sizeof(vec_t));
    *vec = (vec_t) {
        .data_size  = elem_size,
        .len        = 0,
        .first_node = 0,
    };
    return vec;
}

void vec_push(vec_t *vec, uintptr_t addr) {
    if (vec == 0)
        return;

    if (vec->first_node == 0) {
        vec->first_node = (vec_node_t *) kmalloc(sizeof(vec_node_t));
        *vec->first_node = (vec_node_t) {
            .idx        = 0,
            .next       = 0,
            .data_addr  = addr
        };
        vec->len++;
        return;
    }

    vec_node_t *this = (vec_node_t *)vec->first_node;
    while (this->next)
        this = (vec_node_t *)this->next;

    this->next = (uintptr_t) kmalloc(sizeof(vec_node_t));
    *((vec_node_t *) this->next) = (vec_node_t) {
        .idx        = this->idx,
        .next       = 0,
        .data_addr  = addr
    };
    vec->len++;
}

void vec_set(vec_t *vec, size_t idx, uintptr_t val) {
    if (vec->len <= idx || vec == 0)
        return;

    vec_node_t *this = vec->first_node;
    size_t current_idx;

    for (; current_idx < idx; current_idx++)
        this = (vec_node_t *)this->next;
    this->data_addr = val;
}

void *vec_at(vec_t *vec, size_t idx) {
    if (vec->len <= idx || vec == 0)
        return 0;

    vec_node_t *this = vec->first_node;
    size_t current_idx = 0;
    for (; current_idx < idx; current_idx++)
        this = (vec_node_t *)this->next;

    return (void *)this->data_addr;
}

void vec_pop(vec_t *vec, size_t idx) {
    if (vec->len == 0 || vec == 0) {
        return;
    }
    vec_node_t *this = (vec_node_t *)vec->first_node;
    if (idx == 0) {
        vec->first_node = (vec_node_t *)this->next;
        vec->len--;
        return;
    }

    size_t current_idx = 0;
    for (; current_idx < idx; current_idx++)
        this = (vec_node_t *)this->next;

    vec_node_t *next = (vec_node_t *) ((vec_node_t *)this->next)->next;
    this->next = (uintptr_t)next;
    vec->len--;
}

size_t vec_size(vec_t *vec) {
    if (vec == 0)
        return 0;

    size_t idx = 0;
    vec_node_t *this = (vec_node_t *)vec->first_node;

    while (this->next) {
        idx++;
        this = (vec_node_t *)this->next;
    }

    return idx;
}

void vec_free(vec_t *vec) {
    if (vec == 0)
        return;

    vec_node_t *this = vec->first_node;
    vec_node_t *next_node;

    while (this != 0) {
        next_node = (vec_node_t *)this->next;
        kfree(this);
        this = next_node;
    }

    kfree(vec);
}

