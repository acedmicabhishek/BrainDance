#pragma once

extern int ctrl_pressed;
extern unsigned char kbd_us[128];

void keyboard_install();
char keyboard_get_char();
char keyboard_get_char_blocking();
unsigned char keyboard_get_scancode();
