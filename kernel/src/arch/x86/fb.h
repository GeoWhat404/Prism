#pragma once

#include <stdint.h>
#include <boot/limine.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 15

#define COLOR(r, g, b) ((b) | (g << 8) | (r << 16))

void fb_initialize(struct limine_framebuffer *lfb);
void fb_clrscr(void);
void fb_clear_color(uint32_t new_fg, uint32_t new_bg);
void fb_putc(char c);
int fb_get_screen_x(void);
int fb_get_screen_y(void);
void fb_set_screen_x(int x);
void fb_set_screen_y(int y);
