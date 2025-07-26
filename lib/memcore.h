#ifndef MEMCORE_H
#define MEMCORE_H

void* memcpy(void* dest, const void* src, unsigned int count);
void* memset(void* dest, int value, unsigned int count);
int strlen(const char* str);

void print(const char* msg, unsigned char color);
void clear_screen(unsigned char color);
void print_int(int number, unsigned char color);
void print_hex(unsigned int number, unsigned char color);
void print_char(char c, unsigned char color);
void println(const char* msg, unsigned char color);
void kprintf(const char* fmt, ...);


void panic(const char* msg);
void log(const char* tag, const char* msg);
void scroll_up();
#endif
