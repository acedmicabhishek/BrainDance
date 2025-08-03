#ifndef HEAP_H
#define HEAP_H

#include "include/pmm.h"

#define HEAP_START 0x400000
#define HEAP_END   0x800000

void* kmalloc(uint32_t size);
void* alloc_aligned(uint32_t size, uint32_t alignment);
void kfree(void* ptr);

#endif