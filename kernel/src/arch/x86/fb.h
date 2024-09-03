#pragma once

#include <stdint.h>
#include <boot/limine.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 15

#define COLOR(r, g, b) ((b) | (g << 8) | (r << 16))

void fb_initialize(struct limine_framebuffer *lfb);
void fb_clrscr();
void fb_clear_color(uint32_t new_color);
void fb_putc(char c);
int fb_get_screen_x();
int fb_get_screen_y();
void fb_set_screen_x(int x);
void fb_set_screen_y(int y);
void fb_set_cursor(int x, int y);
