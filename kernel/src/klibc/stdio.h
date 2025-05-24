#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <hal/vfs.h>

extern const int stdin;
extern const int stdout;
extern const int stderr;
extern const int stddbg;

void fputc(uint8_t c, fd_t fd);
void fputs(const char *str, fd_t fd);
void vfprintf(fd_t fd, const char *fmt, va_list args);
void fprintf(fd_t fd, const char *fmt, ...);
void fprint_buffer(fd_t fd, const char *msg, const void *buffer, uint32_t count);
int vsnprintf(char *buffer, size_t max_len, const char *fmt, va_list args);

void putc(char c);
void puts(const char *str);
void printf(const char *fmt, ...);
void print_buffer(const char *msg, const void *buffer, uint32_t count);
