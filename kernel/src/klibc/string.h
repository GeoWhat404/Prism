#pragma once

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
void memset(void *_dst, int val, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dest, const void *src, size_t n);
