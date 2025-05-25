%define FONT "font/ter-u18b.psf"
global _font_header
global _font_end
global _font_size
global _font_glyph_table

_font_header:
    incbin FONT, 0, 32

_font_glyph_table:
    incbin FONT, 32

