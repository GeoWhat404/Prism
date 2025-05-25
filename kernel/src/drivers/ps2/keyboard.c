#include "keyboard.h"

#include <hal/irq.h>
#include <hal/port.h>

#include <stdbool.h>

#include <util/logger.h>

static bool enabled;
static bool caps_lock;
static bool shift_pressed;
static pfn_keyboard_callback callback;

#define WAIT() {                                                            \
    uint32_t i;                                                             \
    for(i = 0; (i < 1024) && !is_response_ok(); i++) {                      \
        io_wait();                                                          \
    }                                                                       \
    if(i == 1023)                                                           \
        kerror("Wait: timed out");                                          \
}

static const char scan_code_chars[128] = {
    0,	 27,   '1',	 '2', '3',	'4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',	'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,	  'a',	's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',	 0,	  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,	 '*', 0,	' ', 0,	  0,   0,	0,	 0,	  0,   0,
    0,	 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	'-', 0,	  0,   0,
    '+', 0,	   0,	 0,	  0,	0,	 0,	  0,   0,	0,	 0,	  0};

static void ps2_keyboard_callback(_unused registers_t *unused) {
    if (!enabled)
        return;
    uint8_t scancode = ps2_keyboard_read_scancode();
    if (callback)
        callback(scancode);
}

static char toupper(char ch) {
    if((ch >= 'a') && (ch <= 'z'))
        return ch - 32;
    return ch;
}

static bool is_response_ok() {
    uint8_t status = inb(KBD_STATUS_REG);
    uint8_t ok = status & 1;
    return ok != status;
}

static void flush_output_buffer() {
    while (inb(KBD_STATUS_REG) & 1) {
        inb(KBD_DATA_PORT);
    }
}

static bool test_ps2_first_port() {
    outb(KBD_CMD_REG, 0xAB);
    io_wait();
    WAIT();
    uint8_t in = inb(KBD_DATA_PORT);
    switch(in) {
        case 0x01:
            kerror("PS/2 Port 1: Clock line stuck low");
            return false;

        case 0x02:
            kerror("PS/2 Port 1: Clock line stuck high");
            return false;

        case 0x03:
            kerror("PS/2 Port 1: Data line stuck low");
            return false;

        case 0x04:
            kerror("PS/2 Port 1: Data line stuck high");
            return false;

        case 0x00:
            return true;

        default:
            kerror("PS/2 Port 1: Unknown response: 0x%x", in);
            return false;
    }
}

bool ps2_keyboard_initialize() {
    enabled = true;
    irq_register_handler(IRQ1, ps2_keyboard_callback);

    // disable port 1 & 2 (if exists) to block random
    // junk being sent while testing
    outb(KBD_CMD_REG, KBD_CMD_DISABLE_PORT_1);
    outb(KBD_CMD_REG, KBD_CMD_DISABLE_PORT_2);

    io_wait();

    // flush out random input garbage
    flush_output_buffer();

    /* Test PS/2 Controller */
    outb(KBD_CMD_REG, 0xAA);

    WAIT();
    uint8_t in = inb(KBD_DATA_PORT);
    if(in != KBD_TEST_PASS) {
        kerror("Failed to communicate with the PS/2 Controller. Expected 0x55 but got 0x%x", in);
        return false;
    }

    // re-enable port 1 & 2 (if exists)
    outb(KBD_CMD_REG, KBD_CMD_ENABLE_PORT_1);
    outb(KBD_CMD_REG, KBD_CMD_ENABLE_PORT_2);
    io_wait();
    
    kinfo("PS/2 Keyboard 1 has been initialized");

    /* Test first PS/2 port */
    if(!test_ps2_first_port()) {
        kinfo("PS/2 Port 1: Test failed");
        return false;
    }

    return true;
}

void ps2_keyboard_enable() {
    enabled = true;
}

void ps2_keyboard_disable() {
    enabled = false;
}

bool ps2_keyboard_is_enabled() {
    return enabled;
}

uint8_t ps2_keyboard_read_scancode() {
    WAIT();
    return inb(KBD_DATA_PORT);
}

static char alternate_char(char ch) {
    switch(ch) {
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '-':
            return '_';
        case '=':
            return '+';
        case '[':
            return '{';
        case ']':
            return '}';
        case ';':
            return ':';
        case '"':
            return '\'';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        case '\\':
            return '|';
        case '`':
            return '~';
        default:
            return toupper(ch);
    }
}

char ps2_keyboard_default_proc(int scancode) {
    char ch = 0;

    if (scancode & 0x80) {
        // unmask
        scancode &= ~0x80;

        switch (scancode) {
            case SCAN_CODE_KEY_LEFT_SHIFT:
                shift_pressed = false;
                break;
            default:
                break;
        }

        return 0;
    }

    switch (scancode) {
        case SCAN_CODE_KEY_CAPS_LOCK:
            caps_lock = !caps_lock;
            break;
        case SCAN_CODE_KEY_ENTER:
            ch = '\n';
            break;
        case SCAN_CODE_KEY_TAB:
            ch = '\t';
            break;
        case SCAN_CODE_KEY_LEFT_SHIFT:
            shift_pressed = true;
            break;
        default:
            ch = scan_code_chars[scancode];
            if (caps_lock) {
                ch = (shift_pressed) ? alternate_char(ch) : toupper(ch);
            } else {
                if (shift_pressed) {
                    ch = alternate_char(ch);
                } else {
                    ch = scan_code_chars[scancode];
                }
            }
            break;
    }
    return ch;
}

char ps2_keyboard_read_key() {
    int scancode = ps2_keyboard_read_scancode();
    return ps2_keyboard_default_proc(scancode);
}

void ps2_keyboard_register_callback(pfn_keyboard_callback _callback) {
    callback = _callback;
}
