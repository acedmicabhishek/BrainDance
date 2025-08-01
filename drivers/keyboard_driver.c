#include "include/keyboard.h"
#include "include/ports.h"
#include "include/irq.h"
#include "include/memcore.h"

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

// Scancode to ASCII mapping for a basic US layout
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char last_char = 0;
static unsigned char last_scancode = 0;
int ctrl_pressed = 0;
static int shift_pressed = 0;

void keyboard_handler(struct regs *r) {
    unsigned char scancode;

    // Read from keyboard data port
    scancode = inb(KBD_DATA_PORT);
    last_scancode = scancode;

    // If the top bit of the scancode is set, a key has been released
    if (scancode & 0x80) {
        if (scancode == 0x9D) { // Ctrl release
            ctrl_pressed = 0;
        } else if (scancode == 0xAA || scancode == 0xB6) { // Shift release
            shift_pressed = 0;
        }
    } else {
        // Key pressed
        if (scancode == 0x1D) { // Ctrl press
            ctrl_pressed = 1;
        } else if (scancode == 0x2A || scancode == 0x36) { // Shift press
            shift_pressed = 1;
        } else if (scancode < 128) {
            if (ctrl_pressed) {
                // Send a special code for Ctrl + key
                last_char = 0; // Don't send a normal character
            } else {
                if (shift_pressed) {
                    last_char = kbd_us_shift[scancode];
                } else {
                    last_char = kbd_us[scancode];
                }
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