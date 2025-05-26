#include "shell.h"
#include "graphics/graphics.h"
#include "kernel.h"
#include "util/debug.h"

#include <mem/mem.h>
#include <mem/heap.h>

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <hal/pit.h>
#include <hal/detect.h>
#include <hal/vfs.h>
#include <hal/panic.h>

#include <graphics/gfx.h>
#include <drivers/fb/fb.h>

#include <util/logger.h>
#include <util/colors.h>
#include <drivers/ps2/keyboard.h>

#define BUFFER_SIZE 256

#define IS_COMMAND(buffer, cmd, token_count, tc) ((strcmp((buffer), (cmd)) == 0) && (token_count == tc))
#define CMD_HI "hi"
#define CMD_CLEAR "clear"
#define CMD_EXIT "exit"
#define CMD_CPUDETECT "cpu"
#define CMD_PIT_TIME "pit"
#define CMD_MEM "mem"
#define CMD_MALLOC_TEST "malloc_test"
#define CMD_HELP "help"
#define CMD_PANIC "panic"
#define CMD_GFX_INFO "gfxinfo"

static const char *shell_prompt = "[prism] ";
static bool shell_running = true;
static char *command_buffer;
static int command_buffer_count = 0;

static const char *get_buffer_count_str(enum graphics_buffer_count count) {
    switch (count) {
        case SINGLE: return "SINGLE";
        case DOUBLE: return "DOUBLE";
        case TRIPLE: return "TRIPLE";
        default: return "UNKNOWN";
    }
}

static const char *get_active_buffer_str(enum __graphics_active_buffer buf) {
    switch (buf) {
        case FRAMEBUFFER: return "FRAMEBUFFER";
        case BUFFER0: return "BUFFER0";
        case BUFFER1: return "BUFFER1";
        default: return "UNKNOWN";
    }
}

static const char *get_drawing_mode_str(enum __graphics_drawing_mode mode) {
    switch (mode) {
        case NONE: return "NONE";
        case RECT: return "RECT";
        case ELLIPSE: return "ELLIPSE";
        case TEXT: return "TEXT";
        case LINE: return "LINE";
        default: return "UNKNOWN";
    }
}

static void print_graphics_context() {
    printf(B_GRN"Graphics Context:\n"RES);
    printf(B_GRN"  x_offset:             "B_BLU"%d\n"RES, g_ctx->x_offset);
    printf(B_GRN"  y_offset:             "B_BLU"%d\n"RES, g_ctx->y_offset);
    printf(B_GRN"  ctx_width:            "B_BLU"%d\n"RES, g_ctx->ctx_width);
    printf(B_GRN"  ctx_height:           "B_BLU"%d\n"RES, g_ctx->ctx_height);
    printf(B_GRN"  pitch:                "B_BLU"%u\n"RES, g_ctx->pitch);
    
    printf(B_GRN"  buffer_count:         "B_YEL"%s\n"RES, get_buffer_count_str(g_ctx->buffer_count));
    
    printf(B_GRN"  buffer_size:          "B_BLU"%zu bytes\n"RES, g_ctx->buffer_size);
    printf(B_GRN"  buffer:               "B_GRN"%p\n"RES, g_ctx->buffer);
    printf(B_GRN"  buffer0:              "B_GRN"%p\n"RES, g_ctx->buffer0);
    printf(B_GRN"  buffer1:              "B_GRN"%p\n"RES, g_ctx->buffer1);

    printf(B_GRN"  current_back_buffer:  "B_YEL"%s\n"RES, get_active_buffer_str(g_ctx->current_back_buffer));

    printf(B_GRN"  origin_x:             "B_BLU"%d\n"RES, g_ctx->origin_x);
    printf(B_GRN"  origin_y:             "B_BLU"%d\n"RES, g_ctx->origin_y);

    printf(B_GRN"  x:                    "B_BLU"%d\n"RES, g_ctx->x);
    printf(B_GRN"  y:                    "B_BLU"%d\n"RES, g_ctx->y);
    printf(B_GRN"  w:                    "B_BLU"%d\n"RES, g_ctx->w);
    printf(B_GRN"  h:                    "B_BLU"%d\n"RES, g_ctx->h);

    printf(B_GRN"  line_x:               "B_BLU"%d\n"RES, g_ctx->line_x);
    printf(B_GRN"  line_y:               "B_BLU"%d\n"RES, g_ctx->line_y);
    printf(B_GRN"  line_width:           "B_BLU"%d\n"RES, g_ctx->line_width);

    printf(B_GRN"  stroke_64:            "B_GRN"0x%016llx\n"RES, (unsigned long long)g_ctx->stroke_64);
    printf(B_GRN"  stroke_32:            "B_GRN"0x%08x\n"RES, g_ctx->stroke_32);

    printf(B_GRN"  fill_64:              "B_GRN"0x%016llx\n"RES, (unsigned long long)g_ctx->fill_64);
    printf(B_GRN"  fill_32:              "B_GRN"0x%08x\n"RES, g_ctx->fill_32);

    printf(B_GRN"  mode:                 "B_YEL"%s\n"RES, get_drawing_mode_str(g_ctx->mode));
}

static char **str_split(char* a_str, const char a_delim) {
    char **result    = 0;
    size_t count     = 0;
    char *tmp        = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    count += last_comma < (a_str + strlen(a_str) - 1);
    count++;

    result = kmalloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token) {
            if (idx >= count)
                return 0;

            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        if (idx != count - 1) {
            return 0;
        }
        *(result + idx) = 0;
    }

    return result;
}

static int sstrlen(char **sstr) {
    int i = 0;

    if (!sstr)
        return 0;

    while (sstr[i++]);

    return i - 1;
}

static void print_prompt() {
    printf(B_MAG"%s" WHT, shell_prompt);
}

static void print_help() {
    printf(B_GRN "%s " WHT "-> shows this menu\n", CMD_HELP);
    printf(B_GRN "%s " WHT "-> says hi back\n", CMD_HI);
    printf(B_GRN "%s " WHT "-> clears the screen\n", CMD_CLEAR);
    printf(B_GRN "%s " WHT "-> exits the shell\n", CMD_EXIT);
    printf(B_GRN "%s " WHT "[msg] -> panics with provided message\n", CMD_PANIC);
    printf(B_GRN "%s " WHT "-> shows CPU info\n", CMD_CPUDETECT);
    printf(B_GRN "%s " WHT "-> shows the ticks from the PIT\n", CMD_PIT_TIME);
    printf(B_GRN "%s " WHT "-> prints the memory layout\n", CMD_MEM);
    printf(B_GRN "%s " WHT "-> prints the graphics info\n", CMD_GFX_INFO);
    printf(B_GRN "%s " WHT "-> tests kmalloc(...)\n", CMD_MALLOC_TEST);
}

static void execute_command(char *buffer) {
    if (!shell_running)
        return;

    printf("\n");

    char **tokens = str_split(buffer, ' ');

    if (tokens == 0) {
        goto end;
        return;
    }

    int token_count = sstrlen(tokens);

    if (IS_COMMAND(tokens[0], CMD_HELP, token_count, 1)) {
        print_help();
    } else if (IS_COMMAND(tokens[0], CMD_HI, token_count, 1)) {
        printf("Why hello there!\n");
    } else if (IS_COMMAND(tokens[0], CMD_CLEAR, token_count, 1)) {
        gfx_set_fill(COLOR_BLACK);
        gfx_fill_rect(0, 0, gfx_get_ctx_width(), gfx_get_ctx_height());
        gfx_set_origin(0, 0);
        gfx_rect(0, 0, 0, 0);
        gfx_move_to(0, 0);
        gfx_set_fill(COLOR_BLACK);
        gfx_set_stroke(COLOR_WHITE);
        gfx_set_line_width(4);
        fb_reset();
        gfx_swap_buffer();
    } else if (IS_COMMAND(tokens[0], CMD_EXIT, token_count, 1)) {
        shell_terminate();
        goto end;
    } else if (IS_COMMAND(tokens[0], CMD_CPUDETECT, token_count, 1)) {
        detect_cpu();
    } else if (IS_COMMAND(tokens[0], CMD_PIT_TIME, token_count, 1)) {
        printf("total ticks: %lld\n", pit_get_ticks());
    } else if (IS_COMMAND(tokens[0], CMD_MEM, token_count, 1)) {
        mem_print_layout();
        heap_print();
    } else if (IS_COMMAND(tokens[0], CMD_MALLOC_TEST, token_count, 1)) {
        printf("-- KMALLOC TEST --\n");
        printf("kmalloc'ing 1024 bytes for ptr\n");
        void *ptr = kmalloc(1024);

        if (ptr == 0) {
            printf("kmalloc returned NULL\n");
            printf("-- KMALLOC TEST FAILED --\n");
        } else {
            printf("ptr is at %p\n", ptr);
            heap_print();
            printf("kfree'ing ptr\n");
            kfree(ptr);
            heap_print();

            printf("-- KMALLOC TEST PASSED --\n");
        }
    } else if (IS_COMMAND(tokens[0], CMD_PANIC, token_count, 2)) {
        panic("\"%s\"", tokens[1]);
    } else if (IS_COMMAND(tokens[0], CMD_GFX_INFO, token_count, 1)) {
        print_graphics_context();
    } else {
        if (token_count >= 1)
            printf("Unknown command: \"%s\"\n", tokens[0]);
    }

end:
    if (shell_running)
        print_prompt();

    if (tokens) {
        for (int i = 0; i < sstrlen(tokens); i++) {
            kfree(tokens[i]);
        }

        kfree(tokens);
    }
}

static void keyboard_callback(int scancode) {
    // ignore when a key is released
    switch (scancode) {
        case SCAN_CODE_KEY_BACKSPACE: {

            if (fb_get_cursor_x() == (int32_t) strlen(shell_prompt))
                break;

            // handled by the framebuffer
            printf("\b");

            if (command_buffer_count - 1 >= 0) {
                command_buffer[command_buffer_count--] = '\0';
            }

            break;
        }
        case SCAN_CODE_KEY_ENTER: {
            command_buffer[command_buffer_count] = '\0';
            execute_command(command_buffer);
            memset(command_buffer, 0, BUFFER_SIZE);

            command_buffer_count = 0;

            break;
        }
        case SCAN_CODE_KEY_ESC: {
            ps2_keyboard_disable();
            shell_terminate();
            break;
        }
        default: {
            char ch = ps2_keyboard_default_proc(scancode);
            if (ch != 0) {
                if (command_buffer_count < BUFFER_SIZE - 1) {
                    command_buffer[command_buffer_count++] = ch;
                    printf("%c", ch);
                } else {
                    printf("Command buffer full. Executing command\n");
                    execute_command(command_buffer);
                    memset(command_buffer, 0, BUFFER_SIZE);
                    command_buffer_count = 0;
                }
            }
            break;
        }

    }
}

void shell_launch() {
    shell_running = true;

    kinfo("Launched basic shell");

    printf(B_MAG "PrismOS basic prompt. Type `help` to view commands, <ESC> to exit\n");
    print_prompt();

    command_buffer = kmalloc(BUFFER_SIZE);


    fb_set_cursor(true);
    ps2_keyboard_register_callback(keyboard_callback);

    // stay here
    while (shell_running) { }
}

void shell_terminate() {
    shell_running = false;

    kfree(command_buffer);

    fb_set_cursor(false);
}
