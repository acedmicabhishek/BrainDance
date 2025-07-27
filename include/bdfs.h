#ifndef BDFS_H
#define BDFS_H

#include "pmm.h"

// File Table Entry
typedef struct {
    char     name[32];   // File name
    uint32_t size;       // File size in bytes
    uint32_t offset;     // Offset in RAM disk (from data start)
    uint8_t  used;       // 1 = used, 0 = free
} bdfs_file_t;

// BDFS API
void bdfs_init(uint8_t* ramdisk_base);
int  bdfs_create(const char* name);
int  bdfs_write(const char* name, const void* data, uint32_t size);
int  bdfs_read(const char* name, void* buffer, uint32_t max_size);
int  bdfs_delete(const char* name);
void bdfs_list_files();

#endif // BDFS_H