#pragma once

#include <stddef.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcat(char *s, const char *append);
char *strcpy(char *dst, const char *src);
void strrev(char *s);
void memset(void *_dst, int val, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
char *strtok(char *s, const char *delim);
char *strdup (const char *s);
