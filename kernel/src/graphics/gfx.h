
#pragma once

#include "graphics.h"

#ifndef GFX_CTX
#define GFX_CTX g_ctx
#endif

#define gfx_swap_buffer()                      graphics_swap_buffer(GFX_CTX)
#define gfx_pixel(x, y, color)                 graphics_pixel(GFX_CTX, x, y, color)
#define gfx_draw_char(x, y, c)                 graphics_draw_char(GFX_CTX, x, y, c)
#define gfx_scroll(pixels)                     graphics_scroll(GFX_CTX, pixels)

#define gfx_set_origin(x, y)                   graphics_set_origin(GFX_CTX, x, y)
#define gfx_fill()                             graphics_fill(GFX_CTX)
#define gfx_set_fill(color)                    graphics_set_fill(GFX_CTX, color)
#define gfx_stroke()                           graphics_stroke(GFX_CTX)
#define gfx_set_stroke(color)                  graphics_set_stroke(GFX_CTX, color)
#define gfx_set_line_width(thickness)          graphics_set_line_width(GFX_CTX, thickness)
#define gfx_draw_text(x, y, txt)               graphics_draw_text(GFX_CTX, x, y, txt)
#define gfx_move_to(x, y)                      graphics_move_to(GFX_CTX, x, y)
#define gfx_line_to(x, y)                      graphics_line_to(GFX_CTX, x, y)
#define gfx_rect(x, y, w, h)                   graphics_rect(GFX_CTX, x, y, w, h)
#define gfx_stroke_rect(x, y, w, h)            graphics_stroke_rect(GFX_CTX, x, y, w, h)
#define gfx_fill_rect(x, y, w, h)              graphics_fill_rect(GFX_CTX, x, y, w, h)
#define gfx_clear_rect(x, y, w, h)             graphics_clear_rect(GFX_CTX, x, y, w, h)

#define gfx_get_ctx_width()                    graphics_get_ctx_width(GFX_CTX)
#define gfx_get_ctx_height()                   graphics_get_ctx_height(GFX_CTX)
#define gfx_get_ctx_pitch()                    graphics_get_ctx_pitch(GFX_CTX)

#define gfx_get_font_width()                   graphics_get_font_width()
#define gfx_get_font_height()                  graphics_get_font_height()
#define gfx_get_screen_width()                 graphics_get_screen_width()
#define gfx_get_screen_height()                graphics_get_screen_height()
#define gfx_get_framebuffer()                  graphics_get_framebuffer()


