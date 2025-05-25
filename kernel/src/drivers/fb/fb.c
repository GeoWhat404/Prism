#include <string.h>
#include <util/colors.h>
#include <util/logger.h>
#include <drivers/fb/fb.h>
#include <graphics/graphics.h>

struct __fb_ctx {
	int32_t cursor_x;
	int32_t cursor_y;
	int32_t col_size;
	int32_t row_size;

	graphics_ctx_t *graphics_ctx;
};

static struct __fb_ctx fb;
static uint32_t default_color32 = COLOR(COLOR_WHITE);
static bool enable_cursor = false;

void fb_init(graphics_ctx_t *graphics_ctx, int rows, int cols) {
	fb.cursor_x = 0;
	fb.cursor_y = 0;

	fb.row_size = rows;
	fb.col_size = cols;

    fb.graphics_ctx = graphics_ctx;
}

bool fb_ready() { return fb.row_size ? true : false; }

static void draw_cursor(bool draw) {

    if (!enable_cursor)
        draw = false;

    graphics_set_fill(fb.graphics_ctx, draw ? COLOR_WHITE : COLOR_BLACK);
    graphics_fill_rect(fb.graphics_ctx, fb.cursor_x * graphics_get_font_width(), fb.cursor_y * graphics_get_font_height(),
                       graphics_get_font_width(), graphics_get_font_height());
}

void fb_putc(char c) {
    draw_cursor(false);
	switch (c) {
	case '\t':
		fb.cursor_x += 4;
		break;
	case '\b':
		if (fb.cursor_x > 0 || fb.cursor_y > 0) {
			fb.cursor_x--;
		}
		break;
	case '\n':
		fb.cursor_x = 0;
		fb.cursor_y++;

		break;

	case '\r':
		fb.cursor_y++;
		break;
	default:
		graphics_draw_char(fb.graphics_ctx, fb.cursor_x * graphics_get_font_width(),
				  fb.cursor_y * graphics_get_font_height(), c);

		fb.cursor_x++;
		break;
	}

	if (fb.cursor_x < 0) {
		fb.cursor_x = fb.col_size - 1;
		fb.cursor_y--;
	}

	if (fb.cursor_x > fb.col_size - 1) {
		fb.cursor_x = 0;
		fb.cursor_y++;
	}

	if (fb.cursor_y >= fb.row_size - 2) {
		graphics_scroll(fb.graphics_ctx, graphics_get_font_height());

		fb.cursor_y--;
	}
    draw_cursor(true);
}

void fb_puts(const char *str) {
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\e') {
			if (memcmp(str + i, BLK, 7) == 0 || memcmp(str + i, B_BLK, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_BLACK));
				i += 6;
			} else if (memcmp(str + i, RED, 7) == 0 || memcmp(str + i, B_RED, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_RED));
				i += 6;
			} else if (memcmp(str + i, GRN, 7) == 0 || memcmp(str + i, B_GRN, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_GREEN));
                i += 6;
            } else if (memcmp(str + i, YEL, 7) == 0 || memcmp(str + i, B_YEL, 7) == 0) {
                graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_YELLOW));
                i += 6;
            } else if (memcmp(str + i, BLU, 7) == 0 || memcmp(str + i, B_BLU, 7) == 0) {
                graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_BLUE));
                i += 6;
            } else if (memcmp(str + i, MAG, 7) == 0 || memcmp(str + i, B_MAG, 7) == 0) {
                graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_MAGENTA));
                i += 6;
            } else if (memcmp(str + i, CYN, 7) == 0 || memcmp(str + i, B_CYN, 7) == 0) {
                graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_CYAN));
                i += 6;
            } else if (memcmp(str + i, WHT, 7) == 0 || memcmp(str + i, B_WHT, 7) == 0) {
                graphics_set_stroke(fb.graphics_ctx, COLOR(COLOR_WHITE));
                i += 6;
            } else if (memcmp(str + i, RES, 4) == 0){
                graphics_set_stroke(fb.graphics_ctx, default_color32);
                i += 3;
            }
			continue;
		}
		fb_putc(str[i]);
	}

	graphics_swap_buffer(fb.graphics_ctx);
}

void fb_backspace() {
    graphics_set_fill(fb.graphics_ctx, COLOR_BLACK);
    graphics_fill_rect(fb.graphics_ctx, ((fb.cursor_x - 1) * graphics_get_font_width()), fb.cursor_y * graphics_get_font_height(),
                       graphics_get_font_width(), graphics_get_font_height());
    graphics_swap_buffer(fb.graphics_ctx);

    fb.cursor_x--;
    if (fb.cursor_x < 0) {
        fb.cursor_y--;
        fb.cursor_x = fb.col_size;
    }
}

void fb_reset() {
    fb.cursor_x = 0;
    fb.cursor_y = 0;
}

void fb_set_cursor(bool show) {
    enable_cursor = show;
}

int32_t fb_get_cursor_x() {
    return fb.cursor_x;
}

int32_t fb_get_cursor_y() {
    return fb.cursor_y;
}
