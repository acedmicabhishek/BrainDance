#pragma once

#define _KEY_UP 0x48
#define _KEY_DOWN 0x50

extern int ctrl_pressed;
extern unsigned char kbd_us[128];

void keyboard_install();
char keyboard_get_char();
char keyboard_get_char_blocking();
unsigned char keyboard_get_scancode();
