#pragma once

#define RES "\e[0m"

#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

#define B_BLK "\e[1;30m"
#define B_RED "\e[1;31m"
#define B_GRN "\e[1;32m"
#define B_YEL "\e[1;33m"
#define B_BLU "\e[1;34m"
#define B_MAG "\e[1;35m"
#define B_CYN "\e[1;36m"
#define B_WHT "\e[1;37m"

#define COLOR_BLACK    0x0B0C0D
#define COLOR_RED      0xE06C75
#define COLOR_GREEN    0x98C379
#define COLOR_YELLOW   0xE5C07B
#define COLOR_BLUE     0x61AFEF
#define COLOR_MAGENTA  0xC678DD
#define COLOR_CYAN     0x56B6C2
#define COLOR_WHITE    0xDADADA

#define COLOR_RGB(r, g, b) ((b) | ((g) << 8) | ((r) << 16))

#define GET_R(rgb) (((rgb) >> 16) & 0xFF)
#define GET_G(rgb) (((rgb) >> 8) & 0xFF)
#define GET_B(rgb) ((rgb) & 0xFF)
#define COLOR(rgb) COLOR_RGB(GET_R(rgb), GET_G(rgb), GET_B(rgb))

