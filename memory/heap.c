#include "include/heap.h"

static uint8_t* heap_ptr = (uint8_t*)HEAP_START;

void* kmalloc(uint32_t size) {
    if ((uint32_t)heap_ptr + size > HEAP_END) {
        return 0; // Out of heap space
    }

    void* new_alloc = heap_ptr;
    heap_ptr += size;
    return new_alloc;
}

void* alloc_aligned(uint32_t size, uint32_t alignment) {
    uint32_t addr = (uint32_t)heap_ptr;
    if (addr % alignment != 0) {
        addr = (addr + alignment - 1) & ~(alignment - 1);
    }
    if (addr + size > HEAP_END) {
        return 0;
    }
    heap_ptr = (uint8_t*)(addr + size);
    return (void*)addr;
}

void kfree(void* ptr) {
    // A bump allocator doesn't really have a "free"
    // but we can add a stub for API compatibility.
    (void)ptr; // Avoid unused parameter warning
}