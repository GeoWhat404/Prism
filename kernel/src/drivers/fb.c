#include "fb.h"
#include <string.h>
#include <util/colors.h>
#include <util/logger.h>
#include <graphics/graphics.h>

struct __fb_ctx {
	int32_t cursor_x;
	int32_t cursor_y;
	int32_t col_size;
	int32_t row_size;

	graphics_ctx_t *graphics_ctx;
};

static struct __fb_ctx fb;
static int rdy = 0;

void fb_init(graphics_ctx_t *graphics_ctx, int rows, int cols) {
	fb.cursor_x = 0;
	fb.cursor_y = 0;

	fb.row_size = rows;
	fb.col_size = cols;

    fb.graphics_ctx = graphics_ctx;
}

bool fb_ready(void) { return fb.row_size ? true : false; }

void fb_putc(char c) {
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

	if (fb.cursor_y > fb.row_size - 1) {
		graphics_scroll(fb.graphics_ctx, graphics_get_font_height());

		fb.cursor_y--;
	}
}

void fb_puts(const char *str) {
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\e') {
			if (memcmp(str + i, BLK, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0x000000);
				i += 6;
			} else if (memcmp(str + i, RED, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0xFF0000);
				i += 6;
			} else if (memcmp(str + i, GRN, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0x00FF00);
				i += 6;
			} else if (memcmp(str + i, YEL, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0xFFFF00);
				i += 6;
			} else if (memcmp(str + i, BLU, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0x0000FF);
				i += 6;
			} else if (memcmp(str + i, MAG, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0xFF00FF);
				i += 6;
			} else if (memcmp(str + i, CYN, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0x00FFFF);
				i += 6;
			} else if (memcmp(str + i, WHT, 7) == 0) {
				graphics_set_stroke(fb.graphics_ctx, 0xFFFFFF);
				i += 6;
			}

			continue;
		}

		fb_putc(str[i]);
	}

	graphics_set_stroke(fb.graphics_ctx, 0xFFFFFF);
	graphics_swap_buffer(fb.graphics_ctx);
}
