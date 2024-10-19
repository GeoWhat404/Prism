#pragma once

#include <stddef.h>
#include <stdint.h>

enum graphics_buffer_count { SINGLE, DOUBLE, TRIPLE };

struct graphics_framebuffer {
	void *address;
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	uint8_t red_mask_shift;
	uint8_t red_mask_size;
	uint8_t green_mask_shift;
	uint8_t green_mask_size;
	uint8_t blue_mask_shift;
	uint8_t blue_mask_size;

	struct font *font;

	int valid;
};

enum __graphics_drawing_mode { NONE, RECT, ELLIPSE, TEXT, LINE };

enum __graphics_active_buffer { FRAMEBUFFER, BUFFER0, BUFFER1 };

typedef struct __graphics_context {
    int x_offset;
    int y_offset;
    int ctx_width;
    int ctx_height;
    uint32_t pitch;

    enum graphics_buffer_count buffer_count;

    size_t buffer_size;
    void *buffer;
    void *buffer0;
    void *buffer1;

    enum __graphics_active_buffer current_back_buffer;

    int origin_x;
    int origin_y;

    int x;
    int y;
    int w;
    int h;

    int line_x;
    int line_y;
    int line_width;

    uint64_t stroke_64;
    uint32_t stroke_32;

    uint64_t fill_64;
    uint32_t fill_32;

    enum __graphics_drawing_mode mode;
} graphics_ctx_t;

struct font {
	uint8_t width;
	uint8_t height;
	void (*putc)(graphics_ctx_t *, int, int, char, uint32_t);
};

int graphics_init(struct font *font);

graphics_ctx_t *graphics_get_ctx(enum graphics_buffer_count buffer_count,
								   int x, int y, int width, int height);
int graphics_destroy_ctx(graphics_ctx_t *ctx);

void graphics_swap_buffer(graphics_ctx_t *ctx);
void graphics_pixel(graphics_ctx_t *ctx, int x, int y, uint32_t color);
void graphics_draw_char(graphics_ctx_t *ctx, int x, int y, char c);
void graphics_scroll(graphics_ctx_t *ctx, uint32_t pixels);

void graphics_set_origin(graphics_ctx_t *ctx, int x, int y);
void graphics_fill(graphics_ctx_t *ctx);
void graphics_set_fill(graphics_ctx_t *ctx, uint32_t color);
void graphics_stroke(graphics_ctx_t *ctx);
void graphics_set_stroke(graphics_ctx_t *ctx, uint32_t color);
void graphics_set_line_width(graphics_ctx_t *ctx, uint32_t thickness);
void graphics_draw_text(graphics_ctx_t *ctx, int x, int y, char *txt);
void graphics_move_to(graphics_ctx_t *ctx, int x, int y);
void graphics_line_to(graphics_ctx_t *ctx, int x, int y);
void graphics_rect(graphics_ctx_t *ctx, int x, int y, int w, int h);
void graphics_stroke_rect(graphics_ctx_t *ctx, int x, int y, int w, int h);
void graphics_fill_rect(graphics_ctx_t *ctx, int x, int y, int w, int h);
void graphics_clear_rect(graphics_ctx_t *ctx, int x, int y, int w, int h);

uint8_t graphics_get_font_width();
uint8_t graphics_get_font_height();

uint32_t graphics_get_screen_width();
uint32_t graphics_get_screen_height();

uint32_t graphics_get_ctx_width(graphics_ctx_t *ctx);
uint32_t graphics_get_ctx_height(graphics_ctx_t *ctx);
uint32_t graphics_get_ctx_pitch(graphics_ctx_t *ctx);

struct graphics_framebuffer graphics_get_framebuffer();

