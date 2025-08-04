#include "include/keyboard.h"
#include "include/ports.h"
#include "include/irq.h"
#include "include/memcore.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
char last_char = 0;
static unsigned char last_scancode = 0;
int ctrl_pressed = 0;

void keyboard_handler(struct regs *r) {
    unsigned char scancode;

    scancode = inb(KBD_DATA_PORT);
    last_scancode = scancode;

    if (scancode & 0x80) {
        if (scancode == 0x9D) {
            ctrl_pressed = 0;
        }
    } else {
        if (scancode == 0x1D) {
            ctrl_pressed = 1;
        } else if (scancode == 0x48) {
            last_char = _KEY_UP;
        } else if (scancode == 0x50) {
            last_char = _KEY_DOWN;
        } else if (scancode < 128) {
            if (ctrl_pressed) {
                last_char = 0;
            } else {
                char c = kbd_us[scancode];
                last_char = c;
            }
        }
    }
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}

char keyboard_get_char() {
    if (last_char != 0) {
        char c = last_char;
        last_char = 0;
        return c;
    }
    
    return 0;
}

unsigned char keyboard_get_scancode() {
    while (last_scancode == 0);
    unsigned char sc = last_scancode;
    last_scancode = 0;
    return sc;
}

char keyboard_get_char_blocking() {
    char c;
    do {
        asm volatile("sti");
        asm volatile("hlt");
        c = last_char;
    } while (c == 0);
    last_char = 0;
    return c;
}