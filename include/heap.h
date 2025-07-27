#ifndef HEAP_H
#define HEAP_H

#include "include/pmm.h"

#define HEAP_START 0x400000
#define HEAP_END   0x800000

void* kmalloc(uint32_t size);
void kfree(void* ptr);

#endif