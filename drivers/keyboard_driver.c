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

void keyboard_handler(struct regs *r) {
    unsigned char scancode;

    // Read from keyboard data port
    scancode = inb(KBD_DATA_PORT);

    // If the top bit of the scancode is set, a key has been released
    if (scancode & 0x80) {
        // Handle key release (e.g., for shift, ctrl, alt)
    } else {
        // Key pressed
        if (scancode < 128) {
            char c = kbd_us[scancode];
            if (c != 0) {
                print_char(c, 0x07); // Echo to screen
            }
        }
    }
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}