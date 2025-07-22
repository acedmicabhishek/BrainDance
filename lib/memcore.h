#ifndef MEMCORE_H
#define MEMCORE_H

void* memcpy(void* dest, const void* src, unsigned int count);
void* memset(void* dest, int value, unsigned int count);
int strlen(const char* str);

void print(const char* msg, unsigned char color);

#endif
