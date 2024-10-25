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

// bold
#define B_BLK "\e[1;30m"
#define B_RED "\e[1;31m"
#define B_GRN "\e[1;32m"
#define B_YEL "\e[1;33m"
#define B_BLU "\e[1;34m"
#define B_MAG "\e[1;35m"
#define B_CYN "\e[1;36m"
#define B_WHT "\e[1;37m"

#define COLOR_BLACK   0x3c3836u
#define COLOR_RED     0xcc241du
#define COLOR_GREEN   0x98971au
#define COLOR_YELLOW  0xd79921u
#define COLOR_BLUE    0x458588u
#define COLOR_MAGENTA 0xb16286u
#define COLOR_CYAN    0x689d6au
#define COLOR_WHITE   0xfbf1c7u

#define COLOR_RGB(r, g, b) ((b) | (g << 8) | (r << 16))
#define COLOR(rgb) COLOR_RGB(rgb << 16, rgb << 8, rgb)
