#include "fb.h"
#include <stdio.h>
#include <stdbool.h>
#include <util/debug.h>
#include <util/defines.h>
#include <util/font/font.h>

#define CURSOR_WIDTH FONT_WIDTH
#define CURSOR_HEIGHT FONT_HEIGHT
#define BLINK_INTERVAL 9

#define COLOR(r, g, b) ((b) | (g << 8) | (r << 16))
#define COLOR_BLACK COLOR(0, 0, 0)
#define COLOR_WHITE COLOR(255, 255, 255)
#define COLOR_TEMP(x) COLOR(x * 255, x * 255, x * 255)

static uint32_t *fb;
static uint32_t width;
static uint32_t height;
static uint32_t bpp;
static uint16_t pitch;
static uint32_t screen_x;
static uint32_t screen_y;
static uint32_t cursor_x;
static uint32_t cursor_y;

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
            fb_putpixel(cursor_x + x, cursor_y + y, COLOR_WHITE);
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

void fb_initialize(struct limine_framebuffer *lfb) {
    fb = lfb->address;
    width = lfb->width;
    height = lfb->height;
    bpp = lfb->bpp;
    pitch = lfb->pitch;

    screen_x = 0;
    screen_y = 0;
}

void fb_clrscr() {
    cursor_hide();

    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            if (fb[y * pitch / 4 + x] == 0x00)
                continue;
            fb_putpixel(x, y, 0x00);
        }
    }
    screen_x = 0;
    screen_y = 0;
    fb_set_cursor(screen_x, screen_y);
}

int fb_get_color(int x, int y) {
    return fb[y * pitch / 4 + x];
}

void fb_scrollback(uint32_t lines) {
    cursor_hide();

    /*
    // Ensure lines to scroll is not greater than screen height
    if (lines > screen_y / FONT_HEIGHT) lines = screen_y / FONT_HEIGHT;

    // Calculate the number of bytes per row

    // Move the content up
    for (uint32_t y = lines; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            // Get the pixel color from the current row
            uint32_t color = fb_get_color(x, y);

            // Set the pixel color in the new row
            fb_putpixel(x, y - lines, color);
        }
    }

    // Clear the last 'lines' rows
    for (uint32_t y = height - lines; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            fb_putpixel(x, y, COLOR_BLACK); // Or any other background color
        }
    }

    // Update screen_y position
    screen_y -= lines * FONT_HEIGHT;
    */
    fb_clrscr();
    screen_x = 0;
    screen_y = 0;

    cursor_show();
}


void fb_putc(char c) {
    cursor_hide();

    switch (c) {
        case '\n':
            screen_x = 0;
            screen_y += FONT_HEIGHT;
            goto end;
        case '\r':
            screen_x = 0;
            goto end;
        case '\t':
            for (uint32_t i = 0; i < 8 - (screen_x % 8); i++)
                fb_putc(' ');
            goto end;
    }

    int pos = font_positions[c - 32];
    int byte_width = (FONT_WIDTH - 1) / 8 + 1;

    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint32_t line = 0;
        for (int byte = 0; byte < byte_width; byte++) {
            line = (line << 8) | font_bitmap[pos + row * byte_width + byte];
        }

        for (int col = 0; col < FONT_WIDTH; col++) {
            int pixel = (line & (1 << (FONT_WIDTH - 1 - col))) ? 1 : 0;

            // Epic antialiasing algorithm
            // check neighboring pixels to determine the antialiasing level
            int neighbors = 0;
            if (col > 0) neighbors += (line & (1 << (FONT_WIDTH - col))) ? 1 : 0;
            if (col < FONT_WIDTH - 1) neighbors += (line & (1 << (FONT_WIDTH - 2 - col))) ? 1 : 0;
            if (row > 0) {
                uint32_t prev_line = 0;
                for (int byte = 0; byte < byte_width; byte++) {
                    prev_line = (prev_line << 8) | font_bitmap[pos + (row - 1) * byte_width + byte];
                }
                neighbors += (prev_line & (1 << (FONT_WIDTH - 1 - col))) ? 1 : 0;
                if (col > 0) neighbors += (prev_line & (1 << (FONT_WIDTH - col))) ? 1 : 0;
                if (col < FONT_WIDTH - 1) neighbors += (prev_line & (1 << (FONT_WIDTH - 2 - col))) ? 1 : 0;
            }
            if (row < FONT_HEIGHT - 1) {
                uint32_t next_line = 0;
                for (int byte = 0; byte < byte_width; byte++) {
                    next_line = (next_line << 8) | font_bitmap[pos + (row + 1) * byte_width + byte];
                }
                neighbors += (next_line & (1 << (FONT_WIDTH - 1 - col))) ? 1 : 0;
                if (col > 0) neighbors += (next_line & (1 << (FONT_WIDTH - col))) ? 1 : 0;
                if (col < FONT_WIDTH - 1) neighbors += (next_line & (1 << (FONT_WIDTH - 2 - col))) ? 1 : 0;
            }

            // map the number of set neighbors to a grayscale value
            int grayscale = 0;
            if (pixel) grayscale = 255; // fully on
            else if (neighbors > 6) grayscale = 192; // mostly surrounded
            else if (neighbors > 4) grayscale = 128; // partially surrounded
            else if (neighbors > 2) grayscale = 64;  // slightly surrounded
            else grayscale = 0; // off or no significant neighbors

            fb_putpixel(screen_x + col, screen_y + row, COLOR(grayscale, grayscale, grayscale));
        }
    }

    screen_x += FONT_WIDTH;
end:
    if (screen_x >= width) {
        screen_y += FONT_HEIGHT;
        screen_x = 0;
    }
    if (screen_y >= height) {
        fb_scrollback(1);
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
