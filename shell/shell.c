#include "include/shell.h"
#include "include/memcore.h"
#include "include/keyboard.h"
#include "include/pmm.h"
#include "include/timer.h"
#include "include/bdfs.h"
#include "include/ata.h"
#include "include/colors.h"

#define PROMPT "BD> "
#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_len = 0;

// Command handlers
void help_command() {
    print("Available commands:\n", COLOR_SYSTEM);
    print("  help     - Display this help message\n", COLOR_SYSTEM);
    print("  clear    - Clear the screen\n", COLOR_SYSTEM);
    print("  meminfo  - Show PMM statistics\n", COLOR_SYSTEM);
    print("  time     - Show system uptime\n", COLOR_SYSTEM);
    print("  halt     - Halt the system (requires QEMU to be closed manually)\n", COLOR_SYSTEM);
    print("  ls       - List files\n", COLOR_SYSTEM);
    print("  touch    - Create a file\n", COLOR_SYSTEM);
    print("  write    - Write to a file\n", COLOR_SYSTEM);
    print("  echo     - Print text to the screen\n", COLOR_SYSTEM);
    print("  cat      - Read from a file\n", COLOR_SYSTEM);
    print("  rm       - Remove a file\n", COLOR_SYSTEM);
    print("  mv       - Rename a file\n", COLOR_SYSTEM);
    print("  mkdir    - Create a directory\n", COLOR_SYSTEM);
    print("  cd       - Change directory\n", COLOR_SYSTEM);
    print("  format   - Format the filesystem\n", COLOR_SYSTEM);
    print("  ataread  - Read a sector from the ATA drive\n", COLOR_SYSTEM);
    print("  atawrite - Write a sector to the ATA drive\n", COLOR_SYSTEM);
   }

// Simple atoi implementation
int atoi(const char* str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        res = res * 10 + str[i] - '0';
    }
    return res;
}

void ataread_command(const char* lba_str) {
    if (!lba_str) {
        print("Usage: ataread <lba>\n", COLOR_ERROR);
        return;
    }
    uint32_t lba = atoi(lba_str);
    char buffer[512];
    if (ata_read_sector(lba, buffer) == 0) {
        print("Successfully read sector ", COLOR_ATA_LOG);
        print_int(lba, COLOR_ATA_LOG);
        print(".\n", COLOR_ATA_LOG);
        // Print buffer as hex
        for(int i = 0; i < 512; i++) {
            // This kprintf is fine, it's debug output
            kprintf("%x ", (unsigned char)buffer[i]);
        }
        kprintf("\n");
    } else {
        print("Failed to read sector ", COLOR_ERROR);
        print_int(lba, COLOR_ERROR);
        print(".\n", COLOR_ERROR);
    }
}

void atawrite_command(const char* lba_str, const char* data) {
    if (!lba_str || !data) {
        print("Usage: atawrite <lba> <data>\n", COLOR_ERROR);
        return;
    }
    uint32_t lba = atoi(lba_str);
    char buffer[512];
    memset(buffer, 0, 512);
    strncpy(buffer, data, 511);

    if (ata_write_sector(lba, buffer) == 0) {
        print("Successfully wrote to sector ", COLOR_ATA_LOG);
        print_int(lba, COLOR_ATA_LOG);
        print(".\n", COLOR_ATA_LOG);
    } else {
        print("Failed to write to sector ", COLOR_ERROR);
        print_int(lba, COLOR_ERROR);
        print(".\n", COLOR_ERROR);
    }
}
   
   void ls_command() {
       bdfs_list_files();
   }
   
   void touch_command(const char* filename) {
       if (bdfs_create_file(filename) == 0) {
           print("✔ File '", COLOR_SUCCESS);
           print(filename, COLOR_SUCCESS);
           print("' created.\n", COLOR_SUCCESS);
       } else {
           print("Error creating file '", COLOR_ERROR);
           print(filename, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }
   
   void write_command(const char* filename, const char* data) {
       if (bdfs_write_file(filename, (const uint8_t*)data, strlen(data)) > 0) {
           print("✔ Wrote to file '", COLOR_SUCCESS);
           print(filename, COLOR_SUCCESS);
           print("'.\n", COLOR_SUCCESS);
       } else {
           print("Error writing to file '", COLOR_ERROR);
           print(filename, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }
   
   void cat_command(const char* filename) {
       uint8_t buffer[1024]; // 1KB buffer
       uint32_t bytes_read;
       memset(buffer, 0, 1024);
       if (bdfs_read_file(filename, buffer, &bytes_read) == 0) {
           print("DEBUG: bytes_read = ", COLOR_WARNING);
           print_int(bytes_read, COLOR_WARNING);
           print("\n", COLOR_WARNING);
           // Print hex and ASCII representation
           for (uint32_t i = 0; i < bytes_read; i++) {
               kprintf("%02x ", buffer[i]); // Keep kprintf for hex formatting
           }
           kprintf("\n");
           for (uint32_t i = 0; i < bytes_read; i++) {
               char c = (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.';
               print_char(c, COLOR_FILE);
           }
           kprintf("\n");
       } else {
           print("Error reading from file '", COLOR_ERROR);
           print(filename, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }
   
   void rm_command(const char* filename) {
       if (bdfs_delete_file(filename) == 0) {
           print("✔ File '", COLOR_SUCCESS);
           print(filename, COLOR_SUCCESS);
           print("' deleted.\n", COLOR_SUCCESS);
       } else {
           print("Error deleting file '", COLOR_ERROR);
           print(filename, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }

   void mv_command(const char* old_filename, const char* new_filename) {
       int result = bdfs_rename_file(old_filename, new_filename);
       if (result == 0) {
           print("✔ File '", COLOR_SUCCESS);
           print(old_filename, COLOR_SUCCESS);
           print("' renamed to '", COLOR_SUCCESS);
           print(new_filename, COLOR_SUCCESS);
           print("'.\n", COLOR_SUCCESS);
       } else if (result == -2) {
           print("Error: Source file '", COLOR_ERROR);
           print(old_filename, COLOR_ERROR);
           print("' not found.\n", COLOR_ERROR);
       } else if (result == -3) {
           print("Error: Destination file '", COLOR_ERROR);
           print(new_filename, COLOR_ERROR);
           print("' already exists.\n", COLOR_ERROR);
       } else {
           print("Error renaming file '", COLOR_ERROR);
           print(old_filename, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }

   void mkdir_command(const char* dirname) {
       int result = bdfs_mkdir(dirname);
       if (result == 0) {
           print("✔ Directory '", COLOR_SUCCESS);
           print(dirname, COLOR_SUCCESS);
           print("' created.\n", COLOR_SUCCESS);
       } else if (result == -2) {
           print("Error: '", COLOR_ERROR);
           print(dirname, COLOR_ERROR);
           print("' already exists.\n", COLOR_ERROR);
       } else {
           print("Error creating directory '", COLOR_ERROR);
           print(dirname, COLOR_ERROR);
           print("'.\n", COLOR_ERROR);
       }
   }

   void cd_command(const char* dirname) {
       int result = bdfs_chdir(dirname);
       if (result == -1) {
           print("Error: Directory '", COLOR_ERROR);
           print(dirname, COLOR_ERROR);
           print("' not found.\n", COLOR_ERROR);
       } else if (result == -2) {
           print("Error: '", COLOR_ERROR);
           print(dirname, COLOR_ERROR);
           print("' is not a directory.\n", COLOR_ERROR);
       }
   }

   void format_command() {
       uint8_t buffer[BDFS_FILE_TABLE_SECTORS * 512];
       memset(buffer, 0, sizeof(buffer));
       *(uint32_t*)buffer = BDFS_MAGIC;
       for (int i = 0; i < BDFS_FILE_TABLE_SECTORS; i++) {
           ata_write_sector(BDFS_FILE_TABLE_SECTOR_START + i, buffer + (i * 512));
       }
       bdfs_init();
       print("✔ Filesystem formatted.\n", COLOR_SUCCESS);
   }
   
   void clear_command() {
    clear_screen(0x07);
}

void meminfo_command() {
    print("Physical Memory Manager (PMM) Info:\n", COLOR_SYSTEM);
    print("  Total memory: ", COLOR_SYSTEM);
    print_int(pmm_get_total_memory() / 1024 / 1024, COLOR_SYSTEM);
    print(" MB\n", COLOR_SYSTEM);
    print("  Used memory:  ", COLOR_SYSTEM);
    print_int(pmm_get_used_memory() / 1024 / 1024, COLOR_SYSTEM);
    print(" MB\n", COLOR_SYSTEM);
    print("  Free memory:  ", COLOR_SYSTEM);
    print_int(pmm_get_free_memory() / 1024 / 1024, COLOR_SYSTEM);
    print(" MB\n", COLOR_SYSTEM);
}

void time_command() {
    print("Uptime: ", COLOR_SYSTEM);
    print_int(timer_ticks / 100, COLOR_SYSTEM);
    print(" seconds\n", COLOR_SYSTEM);
}

void halt_command() {
    print("Halting system...\n", COLOR_SYSTEM);
    asm volatile("hlt"); // Halt the CPU
}

void echo_command(const char* text) {
    if (text) {
        print(text, COLOR_INPUT);
        print("\n", COLOR_INPUT);
    }
}

// Function to process a command
void process_command(const char* command) {
    char cmd[MAX_COMMAND_LENGTH];
    strcpy(cmd, command);

    char* token = strtok(cmd, " ");
    
    if (strcmp(token, "help") == 0) {
        help_command();
    } else if (strcmp(token, "clear") == 0) {
        clear_command();
    } else if (strcmp(token, "meminfo") == 0) {
        meminfo_command();
    } else if (strcmp(token, "time") == 0) {
        time_command();
    } else if (strcmp(token, "halt") == 0) {
        halt_command();
    } else if (strcmp(token, "ls") == 0) {
        ls_command();
    } else if (strcmp(token, "touch") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            touch_command(token);
        } else {
            print("Usage: touch <filename>\n", COLOR_ERROR);
        }
    } else if (strcmp(token, "write") == 0) {
        char* filename = strtok(NULL, " ");
        char* data = strtok(NULL, ""); // The rest of the string
        if (filename && data) {
            write_command(filename, data);
        } else {
            print("Usage: write <filename> <data>\n", COLOR_ERROR);
        }
    } else if (strcmp(token, "echo") == 0) {
        char* text = strtok(NULL, "");
        echo_command(text);
    } else if (strcmp(token, "cat") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            cat_command(token);
        } else {
            print("Usage: cat <filename>\n", COLOR_ERROR);
        }
    } else if (strcmp(token, "rm") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            rm_command(token);
        } else {
            print("Usage: rm <filename>\n", COLOR_ERROR);
        }
   } else if (strcmp(token, "mv") == 0) {
       char* old_filename = strtok(NULL, " ");
       char* new_filename = strtok(NULL, " ");
       if (old_filename && new_filename) {
           mv_command(old_filename, new_filename);
       } else {
           print("Usage: mv <source> <destination>\n", COLOR_ERROR);
       }
   } else if (strcmp(token, "mkdir") == 0) {
       char* dirname = strtok(NULL, " ");
       if (dirname) {
           mkdir_command(dirname);
       } else {
           print("Usage: mkdir <dirname>\n", COLOR_ERROR);
       }
   } else if (strcmp(token, "cd") == 0) {
       char* dirname = strtok(NULL, " ");
       if (dirname) {
           cd_command(dirname);
       } else {
           print("Usage: cd <dirname>\n", COLOR_ERROR);
       }
   } else if (strcmp(token, "format") == 0) {
       format_command();
   } else if (strcmp(token, "ataread") == 0) {
       token = strtok(NULL, " ");
       ataread_command(token);
   } else if (strcmp(token, "atawrite") == 0) {
       char* lba_str = strtok(NULL, " ");
       char* data = strtok(NULL, "");
       atawrite_command(lba_str, data);
    } else if (strlen(command) > 0) {
        print("Unknown command: ", COLOR_ERROR);
        print(command, COLOR_ERROR);
        print("\n", COLOR_ERROR);
    }
}

void print_prompt() {
    char dir_name[BDFS_MAX_FILENAME_LENGTH];
    bdfs_get_current_dir_name(dir_name);
    print(dir_name, COLOR_PROMPT);
    print("> ", COLOR_PROMPT);
}

void start_shell() {
    print("Welcome to BrainDance OS\n", COLOR_PROMPT);
    print_prompt();
    while (1) {
        char c = keyboard_get_char();
        if (c == '\0') {
            continue;
        }

        if (c == '\n') {
            command_buffer[command_len] = '\0'; // Null-terminate the command
            print("\n", COLOR_INPUT);
            print("DEBUG: Command buffer: '", COLOR_WARNING);
            print(command_buffer, COLOR_WARNING);
            print("', length: ", COLOR_WARNING);
            print_int(command_len, COLOR_WARNING);
            print("\n", COLOR_WARNING);
            process_command(command_buffer);
            command_len = 0;
            memset(command_buffer, 0, MAX_COMMAND_LENGTH); // Clear buffer
            print_prompt();
        } else if (c == '\b') { // Backspace
            if (command_len > 0) {
                command_len--;
                print_backspace();
            }
        } else {
            if (command_len < MAX_COMMAND_LENGTH - 1) {
                command_buffer[command_len++] = c;
                print_char(c, COLOR_INPUT);
            }
        }
    }
}