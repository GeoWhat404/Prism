#include "font.h"
#include "../graphics/graphics.h"
#include <util/logger.h>

extern char _terminus_font_header;
extern char _terminus_font_glyph_table;

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xb5
#define PSF2_MAGIC2 0x4a
#define PSF2_MAGIC3 0x86

#define PSF2_MAGIC_OK(x) ((x)[0] == PSF2_MAGIC0 && (x)[1] == PSF2_MAGIC1 && (x)[2] == PSF2_MAGIC2 && (x)[3] == PSF2_MAGIC3)

struct psf2_header
{
	char magic[4];
	uint32_t version;
	uint32_t header_size;
	uint32_t flags;
	uint32_t glyph_count;
	uint32_t glyph_size;
	uint32_t glyph_height;
	uint32_t glyph_width;
};

static void _psf2_putc(GRAPHICS_CONTEXT *ctx, int x, int y, char c, uint32_t stroke)
{
	if (c < 0)
	{
		c = 0;
	}

	struct psf2_header *font_header = (struct psf2_header *)&_terminus_font_header;

	uint8_t *glyph = ((uint8_t *)&_terminus_font_glyph_table) + c * font_header->glyph_size;
	uint8_t glyph_line_size = (uint8_t)(font_header->glyph_size / font_header->glyph_height);

	for (uint32_t cy = 0; cy < font_header->glyph_height; cy++)
	{
		for (uint32_t cx = 0; cx < font_header->glyph_width + 1; cx++)
		{
			if (glyph[cx / 8] & (0x80 >> (cx & 7)))
			{
				pixel(ctx, cx + x, cy + y, stroke);
			}
		}

		glyph += glyph_line_size;
	}
}

int psf2_load_font(struct font *font)
{
    struct psf2_header *font_header = (struct psf2_header *)&_terminus_font_header;

    if (PSF2_MAGIC_OK(font_header->magic))
    {
        log_debug("Font magic is valid");
        return -1;
    }

    if (font_header->flags)
    {
        log_debug("Unicode font detected, not supported");
        return -2; // Unicode font is not supported yet!
    }

    log_debug("Loading font: height=%d, width=%d", font_header->glyph_height, font_header->glyph_width);

    font->height = font_header->glyph_height;
    font->width = font_header->glyph_width;
    font->putc = &_psf2_putc;

    log_debug("font->putc points to 0x%016llx", font->putc);

    return 0;
}


