#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

#define isdigit(c) ((c) >= '0' || (c) <= '9')

const char hex_chars[] = "0123456789ABCDEF";

/*
 * As the name suggests this is very sketchy and sucks
 * and should be replaced but i dont care to do it
 */
size_t sketchy_num_len(unsigned long long x) {
    if (x >= 10000000000)   return 11;
    if (x >= 1000000000)    return 10;
    if (x >= 100000000)     return 9;
    if (x >= 10000000)      return 8;
    if (x >= 1000000)       return 7;
    if (x >= 100000)        return 6;
    if (x >= 10000)         return 5;
    if (x >= 1000)          return 4;
    if (x >= 100)           return 3;
    if (x >= 10)            return 2;
    return 1;
}

void append_char(char *buffer, size_t *pos, size_t max_len, char c) {
    if (*pos < max_len - 1) {
        buffer[*pos] = c;
        (*pos)++;
    }
}

void append_str(char *buffer, size_t *pos, size_t max_len, const char *str) {
    while (*str && *pos < max_len - 1) {
        buffer[*pos] = *str;
        (*pos)++;
        str++;
    }
}

void vsnprintf_unsigned(char *buffer, size_t *pos, size_t max_len, unsigned long long number, int radix, int width, char pad_char, bool left_justify) {
    char num_str[32];
    int num_len = 0;

    // Convert number to string
    do {
        int digit = number % radix;
        num_str[num_len++] = hex_chars[digit];
        number /= radix;
    } while (number > 0);

    int pad_len = width > num_len ? width - num_len : 0;

    // Apply padding if right-justified
    if (!left_justify) {
        for (int i = 0; i < pad_len; i++) {
            append_char(buffer, pos, max_len, pad_char);
        }
    }

    // Append the number in reverse order
    while (num_len > 0) {
        append_char(buffer, pos, max_len, num_str[--num_len]);
    }

    // Apply padding if left-justified
    if (left_justify) {
        for (int i = 0; i < pad_len; i++) {
            append_char(buffer, pos, max_len, ' ');
        }
    }
}

void vsnprintf_signed(char *buffer, size_t *pos, size_t max_len, long long number, int radix, int width, char pad_char, bool left_justify) {
    if (number < 0) {
        append_char(buffer, pos, max_len, '-');
        vsnprintf_unsigned(buffer, pos, max_len, -number, radix, width - 1, pad_char, left_justify);
    } else {
        vsnprintf_unsigned(buffer, pos, max_len, number, radix, width, pad_char, left_justify);
    }
}

void vsnprintf_internal(char *buffer, size_t max_len, const char *fmt, va_list args) {
    size_t pos = 0;
    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;
    bool left_justify = false;
    char pad_char = ' ';
    int width = 0;

    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_NORMAL:
                switch (*fmt) {
                    case '%':
                        state = PRINTF_STATE_LENGTH;
                        width = 0;
                        left_justify = false;
                        pad_char = ' ';
                        break;
                    default:
                        append_char(buffer, &pos, max_len, *fmt);
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                if (*fmt == '-') {
                    left_justify = true;
                    fmt++;
                } else if (*fmt == '0') {
                    pad_char = '0';
                    fmt++;
                }

                while (*fmt >= '0' && *fmt <= '9') {
                    width = width * 10 + (*fmt - '0');
                    fmt++;
                }

                switch (*fmt) {
                    case 'h':
                        length = PRINTF_LENGTH_SHORT;
                        state = PRINTF_STATE_LENGTH_SHORT;
                        break;
                    case 'l':
                        length = PRINTF_LENGTH_LONG;
                        state = PRINTF_STATE_LENGTH_LONG;
                        break;
                    default:
                        state = PRINTF_STATE_SPEC;
                        goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_SHORT:
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                } else {
                    state = PRINTF_STATE_SPEC;
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_LENGTH_LONG:
                if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                } else {
                    state = PRINTF_STATE_SPEC;
                    goto PRINTF_STATE_SPEC_;
                }
                break;

            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt) {
                    case 'c': {
                        char c = (char)va_arg(args, int);
                        int pad_len = width > 1 ? width - 1 : 0;

                        if (!left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                append_char(buffer, &pos, max_len, pad_char);
                            }
                        }
                        append_char(buffer, &pos, max_len, c);
                        if (left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                append_char(buffer, &pos, max_len, ' ');
                            }
                        }
                        break;
                    }
                    case 's': {
                        const char *s = va_arg(args, const char *);
                        int len = strlen(s);
                        int pad_len = width > len ? width - len : 0;

                        if (!left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                append_char(buffer, &pos, max_len, pad_char);
                            }
                        }
                        append_str(buffer, &pos, max_len, s);
                        if (left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                append_char(buffer, &pos, max_len, ' ');
                            }
                        }
                        break;
                    }
                    case '%':
                        append_char(buffer, &pos, max_len, '%');
                        break;

                    case 'd':
                    case 'i':
                        radix = 10;
                        sign = true;
                        number = true;
                        break;

                    case 'u':
                        radix = 10;
                        sign = false;
                        number = true;
                        break;

                    case 'X':
                    case 'x':
                    case 'p':
                        radix = 16;
                        sign = false;
                        number = true;
                        break;

                    case 'o':
                        radix = 8;
                        sign = false;
                        number = true;
                        break;

                    default:
                        break;
                }

                if (number) {
                    if (sign) {
                        vsnprintf_signed(buffer, &pos, max_len, va_arg(args, long long), radix, width, pad_char, left_justify);
                    } else {
                        vsnprintf_unsigned(buffer, &pos, max_len, va_arg(args, unsigned long long), radix, width, pad_char, left_justify);
                    }
                }

                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }
        fmt++;
    }

    buffer[pos] = '\0';
}

int vsnprintf(char *buffer, size_t max_len, const char *fmt, va_list args) {
    vsnprintf_internal(buffer, max_len, fmt, args);
    return strlen(buffer);
}

void vfprintf(fd_t fd, const char *fmt, va_list args) {
    char buffer[200];
    size_t written = vsnprintf(buffer, 200, fmt, args);
    vfs_write_s(fd, buffer, written);
}

void fprintf(fd_t fd, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_STDOUT, fmt, args);
    va_end(args);
}

void putc(char c) {
    uint8_t u8 = (uint8_t)c;
    vfs_write(VFS_FD_STDOUT, &u8, 1);
}
