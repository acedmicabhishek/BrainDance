#include "memcore.h"

static int cursor_row = 0;
static int cursor_col = 0;
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((char*)0xB8000)

void* memcpy(void* dest, const void* src, unsigned int count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (unsigned int i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* dest, int value, unsigned int count) {
    char* d = (char*)dest;
    for (unsigned int i = 0; i < count; i++) {
        d[i] = (char)value;
    }
    return dest;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// void clear_screen(unsigned char color) {
//     for (int row = 0; row < VGA_HEIGHT; row++) {
//         for (int col = 0; col < VGA_WIDTH; col++) {
//             int offset = (row * VGA_WIDTH + col) * 2;
//             VGA_MEMORY[offset] = ' ';
//             VGA_MEMORY[offset + 1] = color;
//         }
//     }
//     cursor_row = 0;
//     cursor_col = 0;
// }

// void print_int(int number, unsigned char color) {
//     char buffer[12]; // Enough for 32-bit int: -2147483648
//     int i = 0;
//     int is_negative = 0;

//     if (number == 0) {
//         print("0", color);
//         return;
//     }

//     if (number < 0) {
//         is_negative = 1;
//         number = -number;
//     }
//     while (number != 0) {
//         buffer[i++] = (number % 10) + '0';
//         number /= 10;
//     }

//     if (is_negative) buffer[i++] = '-';

//     // Reverse the buffer
//     for (int j = i - 1; j >= 0; j--) {
//         char c[2] = { buffer[j], 0 };
//         print(c, color);
//     }
// }

// void print_hex(unsigned int number, unsigned char color) {
//     char hex_chars[] = "0123456789ABCDEF";
//     print("0x", color);

//     for (int i = 28; i >= 0; i -= 4) {
//         char digit = hex_chars[(number >> i) & 0xF];
//         char c[2] = { digit, 0 };
//         print(c, color);
//     }
// }


void print(const char* msg, unsigned char color) {
    for (int i = 0; msg[i] != 0; ++i) {
        if (msg[i] == '\n' || cursor_col >= VGA_WIDTH) {
            cursor_row++;
            cursor_col = 0;
            if (msg[i] == '\n') continue;
        }
        if (cursor_row >= VGA_HEIGHT) return;

        int offset = (cursor_row * VGA_WIDTH + cursor_col) * 2;
        VGA_MEMORY[offset] = msg[i];
        VGA_MEMORY[offset + 1] = color;
        cursor_col++;
    }
}
