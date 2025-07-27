#include "../include/bdfs.h"
#include "../include/memcore.h"

#define RAMDISK_SIZE    (4 * 1024 * 1024) // 4 MB
#define SUPERBLOCK_SIZE 512
#define FILETABLE_SIZE  4096
#define MAX_FILES       (FILETABLE_SIZE / sizeof(bdfs_file_t))

// Superblock structure
typedef struct {
    uint32_t magic;
    uint32_t file_count;
} bdfs_superblock_t;

static uint8_t* ramdisk_ptr;
static bdfs_superblock_t* superblock;
static bdfs_file_t* file_table;
static uint8_t* data_blocks_ptr;

void bdfs_init(uint8_t* ramdisk_base) {
    ramdisk_ptr = ramdisk_base;
    superblock = (bdfs_superblock_t*)ramdisk_ptr;
    file_table = (bdfs_file_t*)(ramdisk_ptr + SUPERBLOCK_SIZE);
    data_blocks_ptr = ramdisk_ptr + SUPERBLOCK_SIZE + FILETABLE_SIZE;

    // Initialize superblock
    superblock->magic = 0x42444653; // "BDFS"
    superblock->file_count = 0;

    // Initialize file table
    for (int i = 0; i < MAX_FILES; i++) {
        file_table[i].used = 0;
    }
}

int bdfs_create(const char* name) {
    if (strlen(name) >= 32) {
        return -1; // File name too long
    }

    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, name) == 0) {
            return -2; // File already exists
        }
    }

    // Find a free file table entry
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i].used = 1;
            strncpy(file_table[i].name, name, 31);
            file_table[i].name[31] = '\0';
            file_table[i].size = 0;
            file_table[i].offset = 0; // Will be set on first write
            superblock->file_count++;
            return 0; // Success
        }
    }

    return -3; // No free file table entries
}

int bdfs_write(const char* name, const void* data, uint32_t size) {
    // Find the file
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, name) == 0) {
            // For MVP, we'll just find the next available data block offset
            // A real implementation would need a block allocation strategy
            uint32_t next_offset = 0;
            if (superblock->file_count > 1) {
                 for (int j = 0; j < MAX_FILES; j++) {
                    if(file_table[j].used) {
                        next_offset += file_table[j].size;
                    }
                 }
            }
            
            if (data_blocks_ptr + next_offset + size > ramdisk_ptr + RAMDISK_SIZE) {
                return -2; // Not enough space
            }

            file_table[i].offset = next_offset;
            file_table[i].size = size;
            memcpy(data_blocks_ptr + file_table[i].offset, data, size);
            
            return size; // Success
        }
    }

    return -1; // File not found
}

int bdfs_read(const char* name, void* buffer, uint32_t max_size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, name) == 0) {
            uint32_t read_size = (max_size < file_table[i].size) ? max_size : file_table[i].size;
            memcpy(buffer, data_blocks_ptr + file_table[i].offset, read_size);
            return read_size;
        }
    }
    return -1; // File not found
}

int bdfs_delete(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used && strcmp(file_table[i].name, name) == 0) {
            file_table[i].used = 0;
            superblock->file_count--;
            // In a real FS, we would need to reclaim the data blocks
            return 0; // Success
        }
    }
    return -1; // File not found
}

void bdfs_list_files() {
    kprintf("Listing files in BDFS:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_table[i].used) {
            kprintf("- %s (%d bytes)\n", file_table[i].name, file_table[i].size);
        }
    }
}