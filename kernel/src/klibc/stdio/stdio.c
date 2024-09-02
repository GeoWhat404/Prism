#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <hal/vfs.h>

#include <util/debug.h>

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

const int stdin = VFS_FD_STDIN;
const int stdout = VFS_FD_STDOUT;
const int stderr = VFS_FD_STDERR;
const int stddbg = VFS_FD_DEBUG;

void fputc(char c, fd_t fd) {
    vfs_write(fd, &c, sizeof(c));
}

void fputs(const char *str, fd_t fd) {
    while (*str) {
        fputc(*str, fd);
        str++;
    }
}

const char hex_chars[] = "0123456789ABCDEF";
void fprintf_unsigned(fd_t fd, unsigned long long number, int radix) {
	char buffer[32];
	int pos = 0;

	// convert number to ASCII
	do {
        unsigned long long rem = number % radix;
        number /= radix;
		buffer[pos++] = hex_chars[rem];
	} while(number > 0);

	// print number in reverse order
	while(--pos >= 0)
		fputc(buffer[pos], fd);
}

void fprintf_signed(fd_t fd, long long number, int radix) {
    if (number < 0) {
        fputc('-', fd);
        fprintf_unsigned(fd, number, radix);
        return;
    }
    fprintf_unsigned(fd, number, radix);
}

void vfprintf(fd_t fd, const char *fmt, va_list args) {
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
                        fputc(*fmt, fd);
                        break;
                }
                break;

            case PRINTF_STATE_LENGTH:
                // Handle flags: '-' (left justify) and '0' (zero padding)
                if (*fmt == '-') {
                    left_justify = true;
                    fmt++;
                } else if (*fmt == '0') {
                    pad_char = '0';
                    fmt++;
                }

                // Parse width specifier
                while (*fmt >= '0' && *fmt <= '9') {
                    width = width * 10 + (*fmt - '0');
                    fmt++;
                }

                // After width, expect length specifiers or type specifiers
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
                                fputc(pad_char, fd);
                            }
                        }
                        fputc(c, fd);
                        if (left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                fputc(' ', fd);
                            }
                        }
                        break;
                    }
                    case 's': {
                        const char *s = va_arg(args, const char *);
                        int len = 0;
                        const char *temp = s;
                        while (*temp++) len++;
                        int pad_len = width > len ? width - len : 0;

                        if (!left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                fputc(pad_char, fd);
                            }
                        }
                        fputs(s, fd);
                        if (left_justify) {
                            for (int i = 0; i < pad_len; i++) {
                                fputc(' ', fd);
                            }
                        }
                        break;
                    }
                    case '%':
                        fputc('%', fd);
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
                        // ignore invalid spec
                        break;
                }

                if (number) {
                    // Calculate value length
                    unsigned long long num_value = 0;
                    if (sign) {
                        long long signed_value;
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:
                                signed_value = va_arg(args, int);
                                num_value = signed_value < 0 ? -signed_value : signed_value;
                                break;
                            case PRINTF_LENGTH_LONG:
                                signed_value = va_arg(args, long);
                                num_value = signed_value < 0 ? -signed_value : signed_value;
                                break;
                            case PRINTF_LENGTH_LONG_LONG:
                                signed_value = va_arg(args, long long);
                                num_value = signed_value < 0 ? -signed_value : signed_value;
                                break;
                        }
                    } else {
                        switch (length) {
                            case PRINTF_LENGTH_SHORT_SHORT:
                            case PRINTF_LENGTH_SHORT:
                            case PRINTF_LENGTH_DEFAULT:
                                num_value = va_arg(args, unsigned int);
                                break;
                            case PRINTF_LENGTH_LONG:
                                num_value = va_arg(args, unsigned long);
                                break;
                            case PRINTF_LENGTH_LONG_LONG:
                                num_value = va_arg(args, unsigned long long);
                                break;
                        }
                    }

                    char num_str[32];
                    int num_len = 0;
                    do {
                        int digit = num_value % radix;
                        num_str[num_len++] = digit < 10 ? '0' + digit : 'A' + digit - 10;
                        num_value /= radix;
                    } while (num_value > 0);

                    if (sign && ((signed long long)num_value < 0)) {
                        num_str[num_len++] = '-';
                    }

                    int pad_len = width > num_len ? width - num_len : 0;

                    if (!left_justify) {
                        for (int i = 0; i < pad_len; i++) {
                            fputc(pad_char, fd);
                        }
                    }

                    while (num_len > 0) {
                        fputc(num_str[--num_len], fd);
                    }

                    if (left_justify) {
                        for (int i = 0; i < pad_len; i++) {
                            fputc(' ', fd);
                        }
                    }
                }

                // Reset state
                state = PRINTF_STATE_NORMAL;
                length = PRINTF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }
        fmt++;
    }
}

void fprintf(fd_t fd, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
}

void fprint_buffer(fd_t fd, const char *msg, const void *buffer, uint32_t count) {
    const uint8_t *u8_buffer = (const uint8_t *) buffer;

    fputs(msg, fd);
    for (uint16_t i = 0; i < count; i++) {
        fputc(hex_chars[u8_buffer[i] >> 4], fd);
        fputc(hex_chars[u8_buffer[i] & 0xF], fd);
    }
    fputs("\r\n", fd);
}

void putc(char c) {
    fputc(c, VFS_FD_STDOUT);
}

void puts(const char *str) {
    fputs(str, VFS_FD_STDOUT);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(VFS_FD_STDOUT, fmt, args);

    va_end(args);
}

void print_buffer(const char *msg, const void *buffer, uint32_t count) {
    fprint_buffer(VFS_FD_STDOUT, msg, buffer, count);
}
