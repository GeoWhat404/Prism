#include "fb.h"
#include <stdio.h>
#include <stdbool.h>
#include <util/debug.h>
#include <util/defines.h>
#include <util/font/font.h>

#define CURSOR_WIDTH FONT_WIDTH
#define CURSOR_HEIGHT FONT_HEIGHT
#define BLINK_INTERVAL 9

#define COLOR_TEMP(x) COLOR(x * 255, x * 255, x * 255)

static struct limine_framebuffer *lfb;
static uint32_t *fb;
static uint32_t width;
static uint32_t height;
static uint32_t bpp;
static uint16_t pitch;
static uint32_t screen_x;
static uint32_t screen_y;
static uint32_t cursor_x;
static uint32_t cursor_y;
static uint32_t background_color = COLOR(0, 0, 0);
static uint32_t foreground_color = COLOR(255, 255, 255);

static bool should_show_cursor = false;
static bool cursor_visible = false;
static uint32_t cursor_backup[CURSOR_WIDTH * CURSOR_HEIGHT];
static uint32_t last_blink_ticks = 0;

static void fb_putpixel(uint32_t x, uint32_t y, int color) {
    if (x < width && y < height)
        fb[y * pitch / 4 + x] = color;
}

static void fb_getpixel(uint32_t x, uint32_t y, uint32_t *color) {
    if (x < width && y < height)
        *color = fb[y * pitch / 4 + x];
}

static void cursor_hide() {
    if (!cursor_visible)
        return;

    for (uint32_t y = 0; y < CURSOR_HEIGHT; y++) {
        for (uint32_t x = 0; x < CURSOR_WIDTH; x++) {
            fb_putpixel(cursor_x + x, cursor_y + y, cursor_backup[y * CURSOR_WIDTH + x]);
        }
    }

    cursor_visible = false;
}

static void cursor_show() {
    if (cursor_visible || !should_show_cursor)
        return;

    for (uint32_t y = 0; y < CURSOR_HEIGHT; y++) {
        for (uint32_t x = 0; x < CURSOR_WIDTH; x++) {
            fb_getpixel(cursor_x + x, cursor_y + y, &cursor_backup[y * CURSOR_WIDTH + x]);
            fb_putpixel(cursor_x + x, cursor_y + y, foreground_color);
        }
    }

    cursor_visible = true;
}

static _unused void cursor_callback(uint32_t ticks) {
    if (ticks - last_blink_ticks < BLINK_INTERVAL)
        return;
    last_blink_ticks = ticks;

    if (cursor_visible)
        cursor_hide();
    else
        cursor_show();
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
}

void fb_clrscr() {
    cursor_hide();

    for (uint32_t y = 0; y < lfb->height; y++) {
        for (uint32_t x = 0; x < lfb->width; x++) {
            if (fb[y * pitch / 4 + x] == background_color)
                continue;
            fb_putpixel(x, y, background_color);
        }
    }
    screen_x = 0;
    screen_y = 0;
    fb_set_cursor(screen_x, screen_y);
}

void fb_clear_color(uint32_t new_color) {
    for (uint32_t y = 0; y < lfb->height; y++) {
        for (uint32_t x = 0; x < lfb->width; x++) {
            if (fb[y * pitch / 4 + x] != background_color)
                continue;
            fb_putpixel(x, y, new_color);
        }
    }
    background_color = new_color;
}

int fb_get_color(int x, int y) {
    return fb[y * pitch / 4 + x];
}

void fb_scrollback(uint32_t lines) {
    cursor_hide();

    if (lines > height / FONT_HEIGHT) {
        lines = height / FONT_HEIGHT;
    }

    uint32_t scroll_pixels = lines * FONT_HEIGHT;

    for (uint32_t y = 0; y < height - scroll_pixels; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t color = fb_get_color(x, y + scroll_pixels);
            fb_putpixel(x, y, color);
        }
    }

    for (uint32_t y = height - scroll_pixels; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            fb_putpixel(x, y, background_color);
        }
    }

    screen_y -= scroll_pixels;
    if (screen_y < 0) {
        screen_y = 0;
    }

    cursor_show();
}

void fb_putc(char c) {
    cursor_hide();

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
            // Render the character normally
            int pos = font_positions[c - 32];
            int byte_width = (FONT_WIDTH - 1) / 8 + 1;

            for (int row = 0; row < FONT_HEIGHT; row++) {
                uint32_t line = 0;
                for (int byte = 0; byte < byte_width; byte++) {
                    line = (line << 8) | font_bitmap[pos + row * byte_width + byte];
                }

                for (int col = 0; col < FONT_WIDTH; col++) {
                    int pixel = (line & (1 << (FONT_WIDTH - 1 - col))) ? 1 : 0;
                    int grayscale = pixel ? 255 : 0;
                    fb_putpixel(screen_x + col, screen_y + row, COLOR(grayscale, grayscale, grayscale));
                }
            }

            screen_x += FONT_WIDTH;

            // Check if we need to move to the next line
            if (screen_x + FONT_WIDTH >= width) {
                screen_x = 0;
                screen_y += FONT_HEIGHT;

                // Scroll if the next line exceeds the screen height
                if (screen_y >= height) {
                    fb_scrollback(1);
                    screen_y = height - FONT_HEIGHT;
                }
            }
            break;
        }
    }

    fb_set_cursor(screen_x, screen_y);
    cursor_show();
}

int fb_get_screen_x() {
    return screen_x;
}

int fb_get_screen_y() {
    return screen_y;
}

void fb_set_screen_x(int x) {
    screen_x = x;
}

void fb_set_screen_y(int y) {
    screen_y = y;
}

void fb_set_cursor(int x, int y) {
    cursor_hide();

    cursor_x = x;
    cursor_y = y + FONT_HEIGHT - CURSOR_HEIGHT;

    cursor_show();
}
