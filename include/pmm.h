#ifndef PMM_H
#define PMM_H


typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef signed   int   int32_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;


typedef struct {
    uint64_t base;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) e820_entry_t;


void pmm_init(uint32_t mmap_addr, uint32_t mmap_entries);
void* pmm_alloc_block();
void pmm_free_block(void* block);
uint32_t pmm_get_total_memory();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_free_memory();

#endif