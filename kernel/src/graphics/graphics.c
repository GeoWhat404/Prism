#include <boot/limine.h>
#include <boot/boot.h>
#include <mem/heap.h>
#include <stdint.h>

#include <string.h>
#include <hal/panic.h>
#include <util/logger.h>
#include <util/colors.h>
#include <graphics/graphics.h>

static inline uint64_t color32_to_color64(uint32_t clr) {
    return (((uint64_t)clr) << 32) | (uint64_t)clr;
}

static int g_abs(int n) {
    if (n >= 0)
        return n;

    return n * -1;
}

static struct graphics_framebuffer _framebuffer;

int graphics_init(struct font *font) {

    struct limine_framebuffer *framebuffer = boot_info.lfb;

    _framebuffer.address = framebuffer->address;
    _framebuffer.height = framebuffer->height;
    _framebuffer.width = framebuffer->width;
    _framebuffer.pitch = framebuffer->pitch;
    _framebuffer.bpp = framebuffer->bpp;
    _framebuffer.red_mask_shift = framebuffer->red_mask_shift;
    _framebuffer.red_mask_size = framebuffer->red_mask_size;
    _framebuffer.green_mask_shift = framebuffer->green_mask_shift;
    _framebuffer.green_mask_size = framebuffer->green_mask_size;
    _framebuffer.blue_mask_shift = framebuffer->blue_mask_shift;
    _framebuffer.blue_mask_size = framebuffer->blue_mask_size;

    _framebuffer.font = font;

    _framebuffer.valid = 1;

    return 0;
}

graphics_ctx_t *graphics_get_ctx(enum graphics_buffer_count buffer_count,
                                   int x, int y, int width, int height) {
    if (_framebuffer.valid == 0) {
        return NULL;
    }

    graphics_ctx_t *ctx = kmalloc(sizeof(graphics_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->buffer_count = buffer_count;

    ctx->x_offset = x;
    ctx->y_offset = y;
    ctx->ctx_width = width;
    ctx->ctx_height = height;
    ctx->pitch = width * (_framebuffer.bpp / 8);

    ctx->buffer0 = NULL;
    ctx->buffer1 = NULL;
    ctx->current_back_buffer = FRAMEBUFFER;
    ctx->buffer = _framebuffer.address;
    ctx->buffer_size = width * height * (_framebuffer.bpp / 8);

    graphics_set_origin(ctx, 0, 0);
    graphics_rect(ctx, 0, 0, 0, 0);
    graphics_move_to(ctx, 0, 0);
    graphics_set_fill(ctx, COLOR_BLACK);
    graphics_set_stroke(ctx, COLOR_WHITE);
    graphics_set_line_width(ctx, 4);

    if (buffer_count == DOUBLE) {
        ctx->buffer0 =
            kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));

        if (ctx->buffer0 == NULL) {
            kerror("kmalloc call failed");
            return NULL;
        }

        ctx->buffer1 = NULL;
        ctx->current_back_buffer = BUFFER0;
        ctx->buffer = ctx->buffer0;
    } else if (buffer_count == TRIPLE) {
        ctx->buffer0 =
            kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));
        if (ctx->buffer0 == NULL) {
            return NULL;
        }

        ctx->buffer1 =
            kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));
        if (ctx->buffer0 == NULL) {
            return NULL;
        }

        ctx->current_back_buffer = BUFFER0;
        ctx->buffer = ctx->buffer0;
    } else if (buffer_count != SINGLE) {
        return NULL;
    }

    graphics_clear_rect(ctx, 0, 0, ctx->ctx_width, ctx->ctx_height);
    graphics_swap_buffer(ctx);
    return ctx;
}

int graphics_destroy_ctx(graphics_ctx_t *ctx) {
    if (ctx == NULL)
        return 1;

    if (ctx->buffer_count == DOUBLE) {
        kfree(ctx->buffer0);
    } else if (ctx->buffer_count == TRIPLE) {
        kfree(ctx->buffer0);
        kfree(ctx->buffer1);
    }

    kfree(ctx);

    return 0;
}

void graphics_swap_buffer(graphics_ctx_t *ctx) {
    if (ctx->current_back_buffer == FRAMEBUFFER)
        return;

    uint32_t top_row = ctx->y_offset;
    uint32_t bottom_row = ctx->ctx_height + top_row;

    uint32_t row;

    uint32_t *f_offset = _framebuffer.address + (ctx->pitch * top_row);
    uint32_t *b_offset = ctx->buffer;

    for (row = top_row; row < bottom_row; row++) {
        asm volatile("cld\n"
            "rep movsq" ::"S"(b_offset),
            "D"(f_offset), "c"(ctx->ctx_width / 2));

        f_offset += _framebuffer.width;
        b_offset += ctx->ctx_width;
    }

    if (ctx->buffer_count == DOUBLE)
        return;

    if (ctx->current_back_buffer == BUFFER0) {
        ctx->buffer = ctx->buffer1;
        ctx->current_back_buffer = BUFFER1;
    } else if (ctx->current_back_buffer == BUFFER1) {
        ctx->buffer = ctx->buffer0;
        ctx->current_back_buffer = BUFFER0;
    }
}

void graphics_set_origin(graphics_ctx_t *ctx, int x, int y) {
    ctx->origin_x = x;
    ctx->origin_y = y;
}

void fill(graphics_ctx_t *ctx) {
    if (ctx->mode == RECT) {
        int left = ctx->x + ctx->origin_x;
        int right = left + ctx->w;
        int top = ctx->y + ctx->origin_y;
        int bottom = top + ctx->h;

        int width = ctx->ctx_width;
        int height = ctx->ctx_height;

        if (left < 0)
            left = 0;
        if (left > width)
            left = width;

        if (right < 0)
            right = 0;
        if (right > width)
            right = width;

        if (top < 0)
            top = 0;
        if (top > height)
            top = height;

        if (bottom < 0)
            bottom = 0;
        if (bottom > height)
            bottom = height;

        for (int i = top; i < bottom; i++) {
            uint32_t *offset =
                (uint32_t *)(i * ctx->pitch + (uint64_t)ctx->buffer);

            for (int j = left; j < right; j++)
                offset[j] = ctx->fill_32;
        }
    }
}

void graphics_set_fill(graphics_ctx_t *ctx, uint32_t color) {
    ctx->fill_64 = color32_to_color64(color);
    ctx->fill_32 = color;
}

static void render_line(graphics_ctx_t *ctx, int x0, int y0, int x1, int y1) {
    int dx = g_abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -g_abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    if (sx < 0 || sx >= ctx->ctx_width || sy < 0 || sy >= ctx->ctx_height) {
        return;
    }
    while (1) {
        graphics_pixel(ctx, x0, y0, ctx->stroke_32);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void graphics_stroke(graphics_ctx_t *ctx) {
    int left = ctx->x;
    int top = ctx->y;
    int right = left + ctx->w;
    int bottom = top + ctx->h;

    if (ctx->mode == RECT) {
        graphics_move_to(ctx, left, top);
        graphics_line_to(ctx, right, top);
        graphics_stroke(ctx);
        graphics_line_to(ctx, right, bottom);
        graphics_stroke(ctx);
        graphics_line_to(ctx, left, bottom);
        graphics_stroke(ctx);
        graphics_line_to(ctx, left, top);
        graphics_stroke(ctx);
        ctx->mode = RECT;
        graphics_move_to(ctx, left, top);
    } else if (ctx->mode == LINE) {
        int x0 = ctx->x;
        int y0 = ctx->y;

        int x1 = ctx->line_x;
        int y1 = ctx->line_y;
        for (int i = -ctx->line_width / 2; i < ctx->line_width / 2; i++) {
            render_line(ctx, x0 + i, y0 + i, x1 + i, y1 + i);
        }
    }
}

void graphics_set_stroke(graphics_ctx_t *ctx, uint32_t color) {
    ctx->stroke_64 = color32_to_color64(color);
    ctx->stroke_32 = color;
}

void graphics_set_line_width(graphics_ctx_t *ctx, uint32_t thickness) {
    ctx->line_width = thickness;
}

void graphics_move_to(graphics_ctx_t *ctx, int x, int y) {
    ctx->x = x;
    ctx->y = y;
    ctx->line_x = x;
    ctx->line_y = y;
    ctx->mode = NONE;
}

void graphics_line_to(graphics_ctx_t *ctx, int x, int y) {
    ctx->x = ctx->line_x;
    ctx->y = ctx->line_y;
    ctx->line_x = x;
    ctx->line_y = y;
    ctx->mode = LINE;
    graphics_stroke(ctx);
}

void graphics_rect(graphics_ctx_t *ctx, int x, int y, int w, int h) {
    ctx->x = x;
    ctx->y = y;
    ctx->w = w;
    ctx->h = h;
    ctx->mode = RECT;
}

void graphics_stroke_rect(graphics_ctx_t *ctx, int x, int y, int w, int h) {
    graphics_rect(ctx, x, y, w, h);
    graphics_stroke(ctx);
}

void graphics_fill_rect(graphics_ctx_t *ctx, int x, int y, int w, int h) {
    graphics_rect(ctx, x, y, w, h);
    fill(ctx);
}

void graphics_clear_rect(graphics_ctx_t *ctx, int x, int y, int w, int h) {
    uint32_t color = ctx->fill_32;
    graphics_set_fill(ctx, COLOR_BLACK);
    graphics_fill_rect(ctx, x - ctx->origin_x, y - ctx->origin_y, w, h);
    graphics_set_fill(ctx, color);
}

void graphics_draw_char(graphics_ctx_t *ctx, int x, int y, char c) {
    if (ctx == 0) {
        return;
    }
    _framebuffer.font->putc(ctx, x, y, c, ctx->stroke_32);
}

void graphics_draw_text(graphics_ctx_t *ctx, int x, int y, char *txt) {
    int sx = x + ctx->origin_x;
    int sy = y + ctx->origin_y;

    int i = 0;
    int offset = 0;
    while (txt[i] != '\0') {
        graphics_draw_char(ctx, sx + offset, sy, txt[i]);
        i++;
        offset += graphics_get_font_width();
    }
}

void graphics_pixel(graphics_ctx_t *ctx, int x, int y, uint32_t color) {
    int sx = x + ctx->origin_x;
    int sy = y + ctx->origin_y;

    if (sx < 0 || sx > ctx->ctx_width)
        return;

    if (sy < 0 || sy > ctx->ctx_height)
        return;

    *((uint32_t *)(sy * ctx->pitch + (sx * 4) + (uint64_t)ctx->buffer)) = color;
}

void graphics_scroll(graphics_ctx_t *ctx, uint32_t pixels) {
    if (ctx->buffer_count == SINGLE &&
        graphics_get_ctx_width(ctx) != graphics_get_screen_width()) {
        panic("Not yet implemented!");
    } else if (ctx->buffer_count == DOUBLE) {
        uint32_t size = (ctx->ctx_height - pixels) * ctx->pitch;
        void *src_ptr = ctx->buffer + (pixels * ctx->pitch);
        void *dst_ptr = ctx->buffer;

        asm volatile("cld\n"
            "rep movsq" ::"S"(src_ptr),
            "D"(dst_ptr), "c"(size / 8));

        asm volatile("rep stos" ::"D"(ctx->buffer + size),
            "c"(pixels * ctx->ctx_width), "a"((uint32_t)COLOR_BLACK));
    } else {
        panic("Not yet implemented!");
    }
}

uint8_t graphics_get_font_width() { return _framebuffer.font->width; }

uint8_t graphics_get_font_height() { return _framebuffer.font->height; }

uint32_t graphics_get_screen_width() { return _framebuffer.width; }

uint32_t graphics_get_screen_height() { return _framebuffer.height; }

uint32_t graphics_get_ctx_width(graphics_ctx_t *ctx) { return ctx->ctx_width; }

uint32_t graphics_get_ctx_height(graphics_ctx_t *ctx) { return ctx->ctx_height; }

uint32_t graphics_get_ctx_pitch(graphics_ctx_t *ctx) { return ctx->pitch; }

struct graphics_framebuffer graphics_get_framebuffer() { return _framebuffer; }

