#include "include/bdfs.h"
#include "include/ata.h"
#include "include/memcore.h"
#include "include/colors.h"

static bdfs_file_entry_t file_table[BDFS_MAX_FILES];
static uint32_t current_dir_inode = 0; // Root directory is inode 0

// Helper to find an entry (file or dir) in a specific directory
static int find_entry_in_dir(const char* name, uint32_t parent_inode) {
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] != '\0' && file_table[i].parent_inode == parent_inode && strcmp(file_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Find a free entry in the file table
static int find_free_entry() {
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] == '\0') {
            return i;
        }
    }
    return -1;
}

// Helper to create a directory entry without syncing
static int bdfs_create_dir_entry(const char* dirname, uint32_t parent_inode) {
    if (strlen(dirname) >= BDFS_MAX_FILENAME_LENGTH) return -1;
    if (find_entry_in_dir(dirname, parent_inode) != -1) return -2;

    int free_index = find_free_entry();
    if (free_index == -1) return -3;

    strcpy(file_table[free_index].name, dirname);
    file_table[free_index].type = BDFS_FILE_TYPE_DIRECTORY;
    file_table[free_index].parent_inode = parent_inode;
    file_table[free_index].start_sector = 0; // Not used for dirs
    file_table[free_index].length = 0; // Not used for dirs
    
    return free_index;
}

void bdfs_init() {
    uint8_t buffer[BDFS_FILE_TABLE_SECTORS * 512];
    
    ata_read_sector(BDFS_FILE_TABLE_SECTOR_START, buffer);

    if (*(uint32_t*)buffer != BDFS_MAGIC) {
        kprintf("BDFS: No filesystem found, creating a new one.\n");
        memset(file_table, 0, sizeof(file_table));
        
        // Create root directory at inode 0
        strcpy(file_table[0].name, "/");
        file_table[0].type = BDFS_FILE_TYPE_DIRECTORY;
        file_table[0].parent_inode = 0; // Root's parent is itself
        file_table[0].length = 0;

        // Create default directories
        bdfs_create_dir_entry("soul", 0);
        bdfs_create_dir_entry("cortex", 0);
        int vault_inode = bdfs_create_dir_entry("vault", 0);
        bdfs_create_dir_entry("chrome", 0);
        bdfs_create_dir_entry("drift", 0);
        bdfs_create_dir_entry("ghost", 0);

        if (vault_inode >= 0) {
            bdfs_create_dir_entry("cypher", vault_inode);
        }

        bdfs_sync_file_table();
    } else {
        for (int i = 1; i < BDFS_FILE_TABLE_SECTORS; i++) {
            ata_read_sector(BDFS_FILE_TABLE_SECTOR_START + i, buffer + (i * 512));
        }
        memcpy(file_table, buffer + sizeof(uint32_t), sizeof(file_table));
    }
    current_dir_inode = 0; // Start at the root
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
    if (strlen(filename) >= BDFS_MAX_FILENAME_LENGTH) return -1;
    if (find_entry_in_dir(filename, current_dir_inode) != -1) return -2;

    int free_index = find_free_entry();
    if (free_index == -1) return -3;

    strcpy(file_table[free_index].name, filename);
    file_table[free_index].type = BDFS_FILE_TYPE_FILE;
    file_table[free_index].parent_inode = current_dir_inode;
    file_table[free_index].start_sector = 0;
    file_table[free_index].length = 0;
    
    bdfs_sync_file_table();
    return 0;
}

int bdfs_mkdir(const char* dirname) {
    int result = bdfs_create_dir_entry(dirname, current_dir_inode);
    if (result >= 0) {
        bdfs_sync_file_table();
        return 0;
    }
    return result;
}

int bdfs_delete_file(const char* filename) {
    int file_index = find_entry_in_dir(filename, current_dir_inode);
    if (file_index == -1) return -1;

    // TODO: If it's a directory, ensure it's empty first.
    // For now, we just delete the entry.
    file_table[file_index].name[0] = '\0';
    bdfs_sync_file_table();
    return 0;
}

int bdfs_rename_file(const char* old_filename, const char* new_filename) {
    if (strlen(new_filename) >= BDFS_MAX_FILENAME_LENGTH) return -1;

    int old_file_index = find_entry_in_dir(old_filename, current_dir_inode);
    if (old_file_index == -1) return -2;

    if (find_entry_in_dir(new_filename, current_dir_inode) != -1) return -3;

    strcpy(file_table[old_file_index].name, new_filename);
    bdfs_sync_file_table();
    return 0;
}

void bdfs_list_files() {
    print("Listing for /", COLOR_SYSTEM);
    print(file_table[current_dir_inode].name, COLOR_SYSTEM);
    print(":\n", COLOR_SYSTEM);

    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] != '\0' && file_table[i].parent_inode == current_dir_inode) {
            if (file_table[i].type == BDFS_FILE_TYPE_DIRECTORY) {
                uint8_t color = COLOR_DIR; // Default directory color
                if (strcmp(file_table[i].name, "soul") == 0) color = COLOR_DIR_SOUL;
                else if (strcmp(file_table[i].name, "cortex") == 0) color = COLOR_DIR_CORTEX;
                else if (strcmp(file_table[i].name, "vault") == 0) color = COLOR_DIR_VAULT;
                else if (strcmp(file_table[i].name, "chrome") == 0) color = COLOR_DIR_CHROME;
                else if (strcmp(file_table[i].name, "drift") == 0) color = COLOR_DIR_DRIFT;
                else if (strcmp(file_table[i].name, "ghost") == 0) color = COLOR_DIR_GHOST;
                
                print("d ", color);
                print(file_table[i].name, color);
                print("\n", color);
            } else {
                print("- ", COLOR_FILE);
                print(file_table[i].name, COLOR_FILE);
                print(" (", COLOR_GHOST);
                print_int(file_table[i].length, COLOR_GHOST);
                print(" bytes)\n", COLOR_GHOST);
            }
        }
    }
}

int bdfs_chdir(const char* dirname) {
    if (strcmp(dirname, ".") == 0) {
        return 0; // Stay in the same directory
    }
    if (strcmp(dirname, "..") == 0) {
        if (current_dir_inode != 0) { // Can't go up from root
            current_dir_inode = file_table[current_dir_inode].parent_inode;
        }
        return 0;
    }

    int dir_index = find_entry_in_dir(dirname, current_dir_inode);
    if (dir_index == -1) return -1; // Not found

    if (file_table[dir_index].type != BDFS_FILE_TYPE_DIRECTORY) return -2; // Not a directory

    current_dir_inode = dir_index;
    return 0;
}

uint32_t bdfs_get_current_dir_inode() {
    return current_dir_inode;
}

void bdfs_get_current_dir_name(char* buffer) {
    strcpy(buffer, file_table[current_dir_inode].name);
}

static uint32_t find_free_sector() {
    uint32_t last_sector = BDFS_DATA_SECTOR_START;
    for (int i = 0; i < BDFS_MAX_FILES; i++) {
        if (file_table[i].name[0] != '\0' && file_table[i].type == BDFS_FILE_TYPE_FILE) {
            uint32_t file_end_sector = file_table[i].start_sector + (file_table[i].length + 511) / 512;
            if (file_end_sector > last_sector) {
                last_sector = file_end_sector;
            }
        }
    }
    return last_sector;
}

int bdfs_write_file(const char* filename, const uint8_t* buffer, uint32_t bytes_to_write) {
    int file_index = find_entry_in_dir(filename, current_dir_inode);
    if (file_index == -1) return -1;

    bdfs_file_entry_t* file = &file_table[file_index];
    if (file->type != BDFS_FILE_TYPE_FILE) return -3; // Cannot write to a directory

    uint32_t start_sector;

    if (file->length == 0) {
        start_sector = find_free_sector();
        file->start_sector = start_sector;
    } else {
        start_sector = file->start_sector;
        uint32_t allocated_sectors = (file->length + 511) / 512;
        uint32_t needed_sectors = (bytes_to_write + 511) / 512;
        if (needed_sectors > allocated_sectors) {
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
    int file_index = find_entry_in_dir(filename, current_dir_inode);
    if (file_index == -1) return -1;

    if (file_table[file_index].type != BDFS_FILE_TYPE_FILE) return -2; // Cannot read a directory

    uint32_t start_sector = file_table[file_index].start_sector;
    uint32_t length = file_table[file_index].length;
    *bytes_read = length;

    uint32_t sectors_to_read = (length + 511) / 512;
    for (uint32_t i = 0; i < sectors_to_read; i++) {
        ata_read_sector(start_sector + i, buffer + (i * 512));
    }

    return 0;
}