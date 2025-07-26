#ifndef PAGING_H
#define PAGING_H

#include "pmm.h"

// Page Directory Entry flags
#define PDE_PRESENT  0x1
#define PDE_RW       0x2
#define PDE_USER     0x4

// Page Table Entry flags
#define PTE_PRESENT  0x1
#define PTE_RW       0x2
#define PTE_USER     0x4

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_table_entry_t;

typedef struct {
    page_table_entry_t pages[1024];
} page_table_t;

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t write_thru : 1;
    uint32_t cache_dis  : 1;
    uint32_t accessed   : 1;
    uint32_t unused     : 6;
    uint32_t frame      : 20;
} page_directory_entry_t;

typedef struct {
    page_directory_entry_t tables[1024];
} page_directory_t;

void paging_install();

#endif