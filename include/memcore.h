#ifndef MEMCORE_H
#define MEMCORE_H

#define NULL ((void*)0)

#include "include/types.h"

void* memcpy(void* dest, const void* src, unsigned int count);
void* memmove(void* dest, const void* src, unsigned int count);
void* memset(void* dest, int value, unsigned int count);
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strncpy(char* dest, const char* src, unsigned int n);
int strncmp(const char* str1, const char* str2, size_t n);
char* strtok(char* str, const char* delimiters);
int ends_with(const char* str, const char* suffix);
void print_backspace();

void print(const char* msg, unsigned char color);
void clear_screen(unsigned char color);
void print_int(int number, unsigned char color);
void print_hex(unsigned int number, unsigned char color);
void print_char(char c, unsigned char color);
void println(const char* msg, unsigned char color);
void kprintf(const char* fmt, ...);
int snprintf(char* str, unsigned int size, const char* format, ...);
void print_char_at(char c, unsigned char color, int x, int y);
void set_cursor(int x, int y);

void panic(const char* msg);
void log(const char* tag, const char* msg);
void scroll_up();
void scroll_down();
void update_cursor();
#endif
