#include "include/pmm.h"
#include "include/memcore.h"

#define BLOCK_SIZE 4096
#define BLOCKS_PER_BYTE 8

static uint32_t* pmm_bitmap = 0;
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;

extern uint32_t _kernel_end;

// Helper to set a bit in the bitmap
void set_block(uint32_t bit) {
    pmm_bitmap[bit / 32] |= (1 << (bit % 32));
}

// Helper to clear a bit in the bitmap
void clear_block(uint32_t bit) {
    pmm_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Helper to test a bit in the bitmap
uint32_t test_block(uint32_t bit) {
    return pmm_bitmap[bit / 32] & (1 << (bit % 32));
}

// Helper to find the first free block
int32_t find_first_free() {
    for (uint32_t i = 0; i < total_blocks / 32; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(pmm_bitmap[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;
}

void pmm_init(uint32_t mmap_addr, uint32_t mmap_entries) {
    e820_entry_t* mmap = (e820_entry_t*)mmap_addr;
    uint64_t total_mem = 0;

    for (uint32_t i = 0; i < mmap_entries; i++) {
        if (mmap[i].type == 1) { // Usable RAM
            if (mmap[i].base + mmap[i].len > total_mem) {
                total_mem = mmap[i].base + mmap[i].len;
            }
        }
    }

    total_blocks = total_mem / BLOCK_SIZE;
    pmm_bitmap = (uint32_t*)&_kernel_end;
    uint32_t bitmap_size = total_blocks / BLOCKS_PER_BYTE;

    // Mark all memory as used initially
    memset(pmm_bitmap, 0xFF, bitmap_size);
    used_blocks = total_blocks;

    // Free the usable memory regions
    for (uint32_t i = 0; i < mmap_entries; i++) {
        if (mmap[i].type == 1) { // Usable RAM
            uint64_t base = mmap[i].base;
            uint64_t len = mmap[i].len;
            for (uint64_t j = 0; j < len; j += BLOCK_SIZE) {
                clear_block((base + j) / BLOCK_SIZE);
                used_blocks--;
            }
        }
    }

    // Mark kernel and bitmap as used
    uint32_t kernel_and_bitmap_blocks = ((uint32_t)&_kernel_end + bitmap_size) / BLOCK_SIZE;
    for (uint32_t i = 0; i < kernel_and_bitmap_blocks; i++) {
        if (!test_block(i)) {
            set_block(i);
            used_blocks++;
        }
    }
}

void* pmm_alloc_block() {
    if (used_blocks >= total_blocks) {
        return 0; // Out of memory
    }

    int32_t frame = find_first_free();
    if (frame == -1) {
        return 0; // Should not happen if used_blocks is correct
    }

    set_block(frame);
    used_blocks++;
    return (void*)(frame * BLOCK_SIZE);
}

void pmm_free_block(void* block) {
    uint32_t frame = (uint32_t)block / BLOCK_SIZE;
    if (test_block(frame)) {
        clear_block(frame);
        used_blocks--;
    }
}