#ifndef BDFS_H
#define BDFS_H

#include "pmm.h"

#define BDFS_MAGIC 0x42444653 // "BDFS"
#define BDFS_MAX_FILES 64
#define BDFS_MAX_FILENAME_LENGTH 16
#define BDFS_FILE_TABLE_SECTOR_START 129
#define BDFS_FILE_TABLE_SECTORS 4
#define BDFS_DATA_SECTOR_START (BDFS_FILE_TABLE_SECTOR_START + BDFS_FILE_TABLE_SECTORS)

// Represents a file in the BDFS
typedef enum {
    BDFS_FILE_TYPE_FILE,
    BDFS_FILE_TYPE_DIRECTORY
} bdfs_file_type_t;

typedef struct {
    char name[BDFS_MAX_FILENAME_LENGTH];
    bdfs_file_type_t type;
    uint32_t parent_inode;
    uint32_t start_sector;
    uint32_t length; // Length in bytes for files, number of entries for directories
} bdfs_file_entry_t;

// Function prototypes
void bdfs_init();
void bdfs_sync_file_table();

// File operations
int bdfs_create_file(const char* filename);
int bdfs_delete_file(const char* filename);
int bdfs_rename_file(const char* old_filename, const char* new_filename);
void bdfs_list_files();
int bdfs_read_file(const char* filename, uint8_t* buffer, uint32_t* bytes_read);
int bdfs_write_file(const char* filename, const uint8_t* buffer, uint32_t bytes_to_write);

// Directory operations
int bdfs_mkdir(const char* dirname);
int bdfs_chdir(const char* dirname);
uint32_t bdfs_get_current_dir_inode();
void bdfs_get_current_dir_name(char* buffer);

#endif