#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <graphics/graphics.h>

void fb_init(graphics_ctx_t *graphics_ctx, int rows, int cols);
void fb_putc(char c);
void fb_puts(const char *str);
bool fb_ready();
void fb_reset();
