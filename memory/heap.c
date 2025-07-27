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

void kfree(void* ptr) {
    // A bump allocator doesn't really have a "free"
    // but we can add a stub for API compatibility.
    (void)ptr; // Avoid unused parameter warning
}