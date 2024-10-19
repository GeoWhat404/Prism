#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <graphics/graphics.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 15

#define COLOR(r, g, b) ((b) | (g << 8) | (r << 16))

void fb_init(GRAPHICS_CONTEXT *graphics_ctx, int rows, int cols);
void fb_putc(char c);
void fb_puts(const char *str);
bool fb_ready(void);
