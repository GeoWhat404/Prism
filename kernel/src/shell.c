#include "shell.h"
#include "kernel.h"

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

static const char *shell_prompt = "[prism] ";
static bool shell_running = true;
static char *command_buffer;
static int command_buffer_count = 0;

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

    fprintf(VFS_FD_DEBUG, "token_count=%d\n", token_count);

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
        printf("Exiting shell\n");
        shell_terminate();
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
    } else {
        if (token_count >= 1)
            printf("Unknown command: \"%s\"\n", tokens[0]);
    }

end:
    if (shell_running)
        print_prompt();

    for (int i = 0; *(tokens + i); i++) {
        kfree(*(tokens + i));
    }
    kfree(tokens);
}

static void keyboard_callback(int scancode) {
    // ignore when a key is released
    switch (scancode) {
        case SCAN_CODE_KEY_BACKSPACE: {

            if (fb_get_cursor_x() == strlen(shell_prompt))
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

    printf("PrismOS basic prompt. Type `help` to view commands, "
           "<ESC> to exit\n");
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
