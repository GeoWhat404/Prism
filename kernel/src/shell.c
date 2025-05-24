#include "shell.h"
#include "mem/heap.h"
#include "kernel.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <hal/pit.h>

#include <graphics/gfx.h>
#include <drivers/fb/fb.h>

#include <util/logger.h>
#include <util/colors.h>
#include <drivers/ps2/keyboard.h>

#define BUFFER_SIZE 256

static const char *shell_prompt = "[prism] ";
static bool shell_running = true;
static char *command_buffer;
static int command_buffer_count = 0;
static bool cursor_visible = false;

static int cursor_x = 0, cursor_y = 0;
static char last_cursor_char = ' ';

static void execute_command(char buffer[]) {
    if (!shell_running)
        return;

    printf("\n");

    char *token = strtok(buffer, " ");
    if (token == 0) {
        printf("%s", shell_prompt);
        return;
    }

    if (strcmp(token, "hi") == 0) {
        printf("Why hello there!\n");
    } else if (strcmp(token, "clear") == 0) {
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
    } else if (strcmp(token, "exit") == 0) {
        shell_terminate();
    } else {
        printf("Error!\n");
    }

    //  if (strcmp(token, "hi") == 0) {
    //     printf("Hello there!\n");
    // } else if (strcmp(token, "help") == 0) {
    //     print_help();
    // } else if (strcmp(token, "clear") == 0) {
    //     clrscr();
    // } else if (strcmp(token, "exit") == 0) {
    //     asm("cli\nhlt");
    // } else if (strcmp(token, "panic") == 0) {
    //     panic();
    // } else if (strcmp(token, "mem") == 0) {
    //     print_memory(boot_params->mem_info);
    // } else if (strcmp(token, "timb") == 0) {
    //     printf("Ticks since boot: %d\n", timer_get_ticks());
    //     printf("Seconds since boot: %d\n", timer_get_seconds());
    // } else if (strcmp(token, "time") == 0) {
    //     datetime_t time = rtc_get_datetime();
    //
    //     printf("%d/%d/%d %d:%d:%d\n",
    //            time.days, time.months, time.years,
    //            time.hours, time.minutes, time.seconds);
    // } else if (strcmp(token, "sinfo") == 0) {
    //     printf("BPP: %d\n", boot_params->vbe_mode_info.bpp);
    //     printf("Width: %d\n", screen_width);
    //     printf("Height: %d\n", screen_height);
    //     printf("VBE Version: %u\n", boot_params->vbe_info_block.vbe_version);
    //     printf("VBE Singature: %s\n",
    //            boot_params->vbe_info_block.vbe_signature);
    //     printf("Total memory: %u\n", boot_params->vbe_info_block.total_memory);
    // } else if (strcmp(token, "dbpb") == 0) {
    //     dump_bpb();
    // } else if (strcmp(token, "cpuid") == 0) {
    //     printf("CPU Vendor: %s\n", detect_cpu_vendor());
    // } else {
    //     printf("Unknown or incomplete command `%s`. "
    //            "Type `help` to view available commands.\n",
    //            token);
    // }
    printf("%s", shell_prompt);
}


static void keyboard_callback(int scancode) {
    // ignore when a key is released
    if (scancode & 0x80)
        return;

    switch (scancode) {
        case SCAN_CODE_KEY_BACKSPACE: {
            break;
        }

        case SCAN_CODE_KEY_ENTER: {
            command_buffer[command_buffer_count] = '\0';
            execute_command(command_buffer);
            memset(command_buffer, 0, BUFFER_SIZE);

            last_cursor_char = ' ';
            command_buffer_count = 0;

            break;
        }
        case SCAN_CODE_KEY_ESC: {
            ps2_keyboard_disable();
            shell_running = false;
            break;
        }
        default: {
            char ch = ps2_keyboard_default_proc(scancode);
            if (ch != 0) {
                command_buffer[command_buffer_count++] = ch;

                if (command_buffer_count >= BUFFER_SIZE) {
                    printf("Command buffer full. Executing command\n");
                    execute_command(command_buffer);
                    memset(command_buffer, 0, BUFFER_SIZE);
                }
                printf("%c", ch);
            }
            break;
        }

    }
}

void shell_launch() {
    shell_running = true;

    printf("PrismOS basic prompt. Type `help` to view commands,"
           "<ESC> to exit\n");
    printf("%s", shell_prompt);

    command_buffer = kmalloc(BUFFER_SIZE);

    ps2_keyboard_register_callback(keyboard_callback);

    // stay here
    while (shell_running) { }

    // for the prompt
    printf("\n");
}

void shell_terminate() {
    shell_running = false;

    kfree(command_buffer);
}
