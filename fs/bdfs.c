#include "../include/bdfs.h"
#include "../include/ata.h"
#include "../include/memcore.h"

static bdfs_file_entry_t file_table[BDFS_MAX_FILES];

void bdfs_init() {
    uint8_t buffer[BDFS_FILE_TABLE_SECTORS * 512];
    
    // Read the first sector to check for the magic number
    ata_read_sector(BDFS_FILE_TABLE_SECTOR_START, buffer);

    if (*(uint32_t*)buffer != BDFS_MAGIC) {
        // Filesystem not initialized, create a new one
        kprintf("BDFS: No filesystem found, creating a new one.\n");
        memset(file_table, 0, sizeof(file_table));
        bdfs_sync_file_table(); // This will write the magic number and the empty table
    } else {
        // Filesystem found, load the entire file table
        for (int i = 1; i < BDFS_FILE_TABLE_SECTORS; i++) {
            ata_read_sector(BDFS_FILE_TABLE_SECTOR_START + i, buffer + (i * 512));
        }
        memcpy(file_table, buffer + sizeof(uint32_t), sizeof(file_table));
    }
}

void bdfs_sync_file_table() {
    uint8_t buffer[BDFS_FILE_TABLE_SECTORS * 512];
    memset(buffer, 0, sizeof(buffer));
    *(uint32_t*)buffer = BDFS_MAGIC;
    memcpy(buffer + sizeof(uint32_t), file_table, sizeof(file_table));
    for (int i = 0; i < BDFS_FILE_TABLE_SECTORS; i++) {
        ata_write_sector(BDFS_FILE_TABLE_SECTOR_START + i, buffer + (i * 512));
    }
}

int bdfs_create_file(const char* filename) {
    if (strlen(filename) >= BDFS_MAX_FILENAME_LENGTH) {
        return -1; // Filename too long
    }

    // Check for duplicate filenames
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            return -2; // File already exists
        }
    }

    // Find a free entry
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] == '\0') {
            strcpy(file_table[i].name, filename);
            file_table[i].start_sector = 0;
            file_table[i].length = 0;
            bdfs_sync_file_table();
            return 0; // Success
        }
    }

    return -3; // No free file entries
}

int bdfs_delete_file(const char* filename) {
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            file_table[i].name[0] = '\0';
            bdfs_sync_file_table();
            return 0; // Success
        }
    }
    return -1; // File not found
}

void bdfs_list_files() {
    kprintf("Files:\n");
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] != '\0') {
            kprintf("- %s (%u bytes)\n", file_table[i].name, file_table[i].length);
        }
    }
}

static int find_file(const char* filename) {
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (strcmp(file_table[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

static uint32_t find_free_sector() {
    uint32_t last_sector = BDFS_DATA_SECTOR_START;
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] != '\0') {
            uint32_t file_end_sector = file_table[i].start_sector + (file_table[i].length + 511) / 512;
            if (file_end_sector > last_sector) {
                last_sector = file_end_sector;
            }
        }
    }
    return last_sector;
}

int bdfs_write_file(const char* filename, const uint8_t* buffer, uint32_t bytes_to_write) {
    int file_index = find_file(filename);
    if (file_index == -1) {
        return -1; // File not found
    }

    bdfs_file_entry_t* file = &file_table[file_index];
    uint32_t start_sector;

    // If file is new, allocate sectors. Otherwise, reuse existing ones.
    if (file->length == 0) {
        start_sector = find_free_sector();
        file->start_sector = start_sector;
    } else {
        start_sector = file->start_sector;
        // Check if new data fits in the allocated space.
        uint32_t allocated_sectors = (file->length + 511) / 512;
        uint32_t needed_sectors = (bytes_to_write + 511) / 512;
        if (needed_sectors > allocated_sectors) {
            // This simple filesystem does not support growing files.
            return -2; // Not enough space
        }
    }

    file->length = bytes_to_write;

    uint32_t sectors_to_write = (bytes_to_write + 511) / 512;
    for (uint32_t i = 0; i < sectors_to_write; i++) {
        ata_write_sector(start_sector + i, buffer + (i * 512));
    }

    bdfs_sync_file_table();
    return bytes_to_write;
}

int bdfs_read_file(const char* filename, uint8_t* buffer, uint32_t* bytes_read) {
    int file_index = find_file(filename);
    if (file_index == -1) {
        return -1; // File not found
    }

    uint32_t start_sector = file_table[file_index].start_sector;
    uint32_t length = file_table[file_index].length;
    *bytes_read = length;

    uint32_t sectors_to_read = (length + 511) / 512;
    for (uint32_t i = 0; i < sectors_to_read; i++) {
        ata_read_sector(start_sector + i, buffer + (i * 512));
    }

    return 0;
}