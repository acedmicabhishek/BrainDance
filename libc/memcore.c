#include "./include/memcore.h"

static int cursor_row = 0;
static int cursor_col = 0;
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((char*)0xB8000)

typedef char* va_list;

#define va_start(ap, last_arg) (ap = (char*)(&last_arg + 1))
#define va_arg(ap, type)       (*(type*)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap)             (ap = 0)


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

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

void print_backspace() {
    if (cursor_col > 0) {
        cursor_col--;
        int offset = (cursor_row * VGA_WIDTH + cursor_col) * 2;
        VGA_MEMORY[offset] = ' ';
        VGA_MEMORY[offset + 1] = 0x07; // Default color
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = VGA_WIDTH - 1;
        int offset = (cursor_row * VGA_WIDTH + cursor_col) * 2;
        VGA_MEMORY[offset] = ' ';
        VGA_MEMORY[offset + 1] = 0x07; // Default color
    }
}

void clear_screen(unsigned char color) {
    for (int row = 0; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            int offset = (row * VGA_WIDTH + col) * 2;
            VGA_MEMORY[offset] = ' ';
            VGA_MEMORY[offset + 1] = color;
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

void print_int(int number, unsigned char color) {
    char buffer[12]; // Enough for 32-bit int: -2147483648
    int i = 0;
    int is_negative = 0;

    if (number == 0) {
        print("0", color);
        return;
    }

    if (number < 0) {
        is_negative = 1;
        number = -number;
    }
    while (number != 0) {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

    if (is_negative) buffer[i++] = '-';

    // Reverse the buffer
    for (int j = i - 1; j >= 0; j--) {
        char c[2] = { buffer[j], 0 };
        print(c, color);
    }
}

void print_hex(unsigned int number, unsigned char color) {
    char hex_chars[] = "0123456789ABCDEF";
    print("0x", color);

    for (int i = 28; i >= 0; i -= 4) {
        char digit = hex_chars[(number >> i) & 0xF];
        char c[2] = { digit, 0 };
        print(c, color);
    }
}


void print(const char* msg, unsigned char color) {
    for (int i = 0; msg[i] != 0; ++i) {
        if (msg[i] == '\n' || cursor_col >= VGA_WIDTH) {
            cursor_row++;
            cursor_col = 0;
            if (msg[i] == '\n') continue;
        }
        if (cursor_row >= VGA_HEIGHT) scroll_up();

        int offset = (cursor_row * VGA_WIDTH + cursor_col) * 2;
        VGA_MEMORY[offset] = msg[i];
        VGA_MEMORY[offset + 1] = color;
        cursor_col++;
    }
}

void scroll_up() {
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            int from = (row * VGA_WIDTH + col) * 2;
            int to = ((row - 1) * VGA_WIDTH + col) * 2;
            VGA_MEMORY[to] = VGA_MEMORY[from];
            VGA_MEMORY[to + 1] = VGA_MEMORY[from + 1];
        }
    }

    // Clear last line
    for (int col = 0; col < VGA_WIDTH; col++) {
        int offset = ((VGA_HEIGHT - 1) * VGA_WIDTH + col) * 2;
        VGA_MEMORY[offset] = ' ';
        VGA_MEMORY[offset + 1] = 0x07;
    }

    cursor_row = VGA_HEIGHT - 1;
}

void panic(const char* msg) {
    clear_screen(0x4F);  // Red bg, white fg
    print("!!! KERNEL PANIC !!!\n\n", 0x4F);
    print(msg, 0x4F);
    while (1) {
        __asm__ volatile ("cli; hlt");
    }
}

void log(const char* tag, const char* msg) {
    print("[", 0x07);
    print(tag, 0x07);
    print("] ", 0x07);
    print(msg, 0x07);
    print("\n", 0x07);
}

void print_char(char c, unsigned char color) {
    if (c == '\n' || cursor_col >= VGA_WIDTH) {
        cursor_row++;
        cursor_col = 0;
        if (c == '\n') return;
    }

    if (cursor_row >= VGA_HEIGHT) {
        scroll_up();
    }

    int offset = (cursor_row * VGA_WIDTH + cursor_col) * 2;
    VGA_MEMORY[offset] = c;
    VGA_MEMORY[offset + 1] = color;
    cursor_col++;
}

void println(const char* msg, unsigned char color) {
    print(msg, color);
    print_char('\n', color);
}


void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%') {
            i++;
            char spec = fmt[i];
            switch (spec) {
                case 'd': {
                    int val = va_arg(args, int);
                    print_int(val, 0x07);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_hex(val, 0x07);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    print(str, 0x07);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int); // promote to int
                    print_char(c, 0x07);
                    break;
                }
                case '%': {
                    print_char('%', 0x07);
                    break;
                }
                default:
                    print_char('%', 0x07);
                    print_char(spec, 0x07);
                    break;
            }
        } else {
            print_char(fmt[i], 0x07);
        }
    }

    va_end(args);
}
