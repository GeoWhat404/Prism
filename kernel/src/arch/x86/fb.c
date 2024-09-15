#include "fb.h"
#include "pit.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <util/debug.h>
#include <util/defines.h>
#include <util/font/font.h>

#define COLOR_TEMP(x) COLOR(x * 255, x * 255, x * 255)

static uint32_t *fb;
static uint32_t width;
static uint32_t height;
static uint32_t bpp;
static uint16_t pitch;
static uint32_t screen_x;
static uint32_t screen_y;
static uint32_t background_color = COLOR(0, 0, 0);
static uint32_t foreground_color = COLOR(255, 255, 255);
static struct limine_framebuffer *lfb;

static uint32_t max_lines = 0;
static uint32_t max_columns = 0;

static inline void fb_putpixel(uint32_t x, uint32_t y, int color) {
    if (x < width && y < height)
        fb[y * pitch / 4 + x] = color;
}

static inline void fb_getpixel(uint32_t x, uint32_t y, uint32_t *color) {
    if (x < width && y < height)
        *color = fb[y * pitch / 4 + x];
}

void fb_initialize(struct limine_framebuffer *_lfb) {
    lfb = _lfb;
    fb = lfb->address;
    width = lfb->width - (lfb->width % FONT_WIDTH);
    height = lfb->height - (lfb->height % FONT_HEIGHT);
    bpp = lfb->bpp;
    pitch = lfb->pitch;

    screen_x = 0;
    screen_y = 0;

    background_color = COLOR(0, 0, 0);
    foreground_color = COLOR(255, 255, 255);
}

void fb_clrscr(void) {
    uint32_t *fb_end = fb + (lfb->pitch / 4) * lfb->height;
    for (uint32_t *pixel = fb; pixel < fb_end; pixel++) {
        *pixel = background_color;
    }

    screen_x = 0;
    screen_y = 0;
}

void fb_clear_color(uint32_t new_fg, uint32_t new_bg) {
    uint32_t *framebuffer_end = fb + (lfb->pitch / 4) * lfb->height;
    for (uint32_t *pixel = fb; pixel < framebuffer_end; pixel++) {
        if (*pixel == background_color) {
            *pixel = new_bg;
        } else {
            *pixel = new_fg;
        }
    }

    background_color = new_bg;
    foreground_color = new_fg;
}

int fb_get_color(int x, int y) {
    return fb[y * pitch / 4 + x];
}

void fb_scrollback(uint32_t lines) {
    if (lines > height / FONT_HEIGHT) {
        lines = height / FONT_HEIGHT;
    }

    uint32_t scroll_pixels = lines * FONT_HEIGHT;
    uint32_t *src = fb + (scroll_pixels * (pitch / 4));
    uint32_t *dest = fb;

    size_t bytes_to_copy = (height - scroll_pixels) * pitch;

    memmove(dest, src, bytes_to_copy);

    uint32_t *clear_start = fb + ((height - scroll_pixels) * (pitch / 4));
    memset(clear_start, background_color, scroll_pixels * (pitch / 4) * sizeof(uint32_t));

    screen_y -= scroll_pixels;
    if (screen_y < 0) {
        screen_y = 0;
    }
}

void fb_putc(char c) {
    switch (c) {
        case '\n':
            screen_x = 0;
            screen_y += FONT_HEIGHT;

            if (screen_y >= height) {
                fb_scrollback(1);
                screen_y = height - FONT_HEIGHT;
            }
            break;

        case '\r':
            screen_x = 0;
            break;

        case '\t':
            for (uint32_t i = 0; i < 4 - (screen_x % 4); i++)
                fb_putc(' ');
            return;

        default: {
            int pos = font_positions[c - 32];
            int byte_width = (FONT_WIDTH - 1) / 8 + 1;

            for (int row = 0; row < FONT_HEIGHT; row++) {
                uint32_t line = 0;
                for (int byte = 0; byte < byte_width; byte++) {
                    line = (line << 8) | font_bitmap[pos + row * byte_width + byte];
                }

                for (int col = 0; col < FONT_WIDTH; col++) {
                    int pixel = (line & (1 << (FONT_WIDTH - 1 - col))) ? 1 : 0;
                    int color = pixel ? foreground_color : background_color;
                    fb_putpixel(screen_x + col, screen_y + row, color);
                }
            }

            screen_x += FONT_WIDTH;

            if (screen_x + FONT_WIDTH >= width) {
                screen_x = 0;
                screen_y += FONT_HEIGHT;

                if (screen_y >= height) {
                    fb_scrollback(1);
                    screen_y = height - FONT_HEIGHT;
                }
            }
            break;
        }
    }
}

int fb_get_screen_x(void) {
    return screen_x;
}

int fb_get_screen_y(void) {
    return screen_y;
}

void fb_set_screen_x(int x) {
    screen_x = x;
}

void fb_set_screen_y(int y) {
    screen_y = y;
}
