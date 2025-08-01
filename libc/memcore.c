#include "./include/memcore.h"
#include "./include/ports.h"
#include "./include/types.h"

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

void* memmove(void* dest, const void* src, unsigned int count) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d < s) {
        for (unsigned int i = 0; i < count; i++) {
            d[i] = s[i];
        }
    } else {
        for (unsigned int i = count; i != 0; i--) {
            d[i - 1] = s[i - 1];
        }
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

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0;
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

void print_uint(unsigned int number, unsigned char color) {
    char buffer[11]; // Enough for 32-bit unsigned int
    int i = 0;

    if (number == 0) {
        print("0", color);
        return;
    }

    while (number != 0) {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

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
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_uint(val, 0x07);
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

char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
    return original_dest;
}

static char *strtok_state = NULL;

// Helper to check if a character is a delimiter
static int is_delimiter(char c, const char* delimiters) {
    while (*delimiters) {
        if (c == *delimiters++) {
            return 1;
        }
    }
    return 0;
}

char* strtok(char* str, const char* delimiters) {
    if (str != NULL) {
        strtok_state = str;
    }

    if (strtok_state == NULL || *strtok_state == '\0') {
        return NULL;
    }

    // Skip leading delimiters
    char* token_start = strtok_state;
    while (is_delimiter(*token_start, delimiters)) {
        token_start++;
    }

    if (*token_start == '\0') {
        strtok_state = token_start;
        return NULL;
    }

    // Find the end of the token
    char* token_end = token_start;
    while (*token_end && !is_delimiter(*token_end, delimiters)) {
        token_end++;
    }

    if (*token_end != '\0') {
        *token_end = '\0';
        strtok_state = token_end + 1;
    } else {
        strtok_state = token_end;
    }

    return token_start;
}

char* strncpy(char* dest, const char* src, unsigned int n) {
    unsigned int i;
    char* original_dest = dest;

    for (i = 0; i < n && *src != '\0'; i++) {
        *dest++ = *src++;
    }

    // If we haven't filled n characters, pad with nulls
    for ( ; i < n; i++) {
        *dest++ = '\0';
    }

    return original_dest;
   }
   
   int ends_with(const char* str, const char* suffix) {
       if (!str || !suffix) {
           return 0;
       }
       size_t str_len = strlen(str);
       size_t suffix_len = strlen(suffix);
       if (suffix_len > str_len) {
           return 0;
       }
       return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
   }
   
   // A very basic snprintf that only handles %s, %c, and %d
   int snprintf(char* str, unsigned int size, const char* format, ...) {
       va_list args;
       va_start(args, format);
   
       unsigned int written = 0;
       for (int i = 0; format[i] != '\0' && written < size - 1; i++) {
           if (format[i] == '%') {
               i++;
               switch (format[i]) {
                   case 'c': {
                       char c = (char)va_arg(args, int);
                       str[written++] = c;
                       break;
                   }
                   case 's': {
                       const char* s = va_arg(args, const char*);
                       while (*s && written < size - 1) {
                           str[written++] = *s++;
                       }
                       break;
                   }
                   case 'd': {
                       int n = va_arg(args, int);
                       char buf[12];
                       int j = 0;
                       if (n < 0) {
                           str[written++] = '-';
                           n = -n;
                       }
                       do {
                           buf[j++] = (n % 10) + '0';
                           n /= 10;
                       } while (n > 0);
                       while (j > 0 && written < size - 1) {
                           str[written++] = buf[--j];
                       }
                       break;
                   }
               }
           } else {
               str[written++] = format[i];
           }
       }
   
       str[written] = '\0';
       va_end(args);
       return written;
   }
   
   void print_char_at(char c, unsigned char color, int x, int y) {
       if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
       int offset = (y * VGA_WIDTH + x) * 2;
       VGA_MEMORY[offset] = c;
       VGA_MEMORY[offset + 1] = color;
   }
   
   void set_cursor(int x, int y) {
       if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
       cursor_col = x;
       cursor_row = y;
       uint16_t pos = y * VGA_WIDTH + x;
       outb(0x3D4, 0x0F);
       outb(0x3D5, (uint8_t)(pos & 0xFF));
       outb(0x3D4, 0x0E);
       outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
   }
