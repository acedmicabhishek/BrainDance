#include "include/shell.h"
#include "include/memcore.h"
#include "include/keyboard.h"
#include "include/pmm.h"
#include "include/timer.h"
#include "include/bdfs.h"
#include "include/ata.h"

#define PROMPT "BD> "
#define MAX_COMMAND_LENGTH 256

static char command_buffer[MAX_COMMAND_LENGTH];
static int command_len = 0;

// Command handlers
void help_command() {
    print("Available commands:\n", 0x07);
    print("  help     - Display this help message\n", 0x07);
    print("  clear    - Clear the screen\n", 0x07);
    print("  meminfo  - Show PMM statistics\n", 0x07);
    print("  time     - Show system uptime\n", 0x07);
    print("  halt     - Halt the system (requires QEMU to be closed manually)\n", 0x07);
    print("  ls       - List files\n", 0x07);
    print("  touch    - Create a file\n", 0x07);
    print("  write    - Write to a file\n", 0x07);
    print("  cat      - Read from a file\n", 0x07);
    print("  rm       - Remove a file\n", 0x07);
    print("  format   - Format the filesystem\n", 0x07);
    print("  ataread  - Read a sector from the ATA drive\n", 0x07);
    print("  atawrite - Write a sector to the ATA drive\n", 0x07);
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
        print("Usage: ataread <lba>\n", 0x04);
        return;
    }
    uint32_t lba = atoi(lba_str);
    char buffer[512];
    if (ata_read_sector(lba, buffer) == 0) {
        kprintf("Successfully read sector %d.\n", lba);
        // Print buffer as hex
        for(int i = 0; i < 512; i++) {
            kprintf("%x ", (unsigned char)buffer[i]);
        }
        kprintf("\n");
    } else {
        kprintf("Failed to read sector %d.\n", lba);
    }
}

void atawrite_command(const char* lba_str, const char* data) {
    if (!lba_str || !data) {
        print("Usage: atawrite <lba> <data>\n", 0x04);
        return;
    }
    uint32_t lba = atoi(lba_str);
    char buffer[512];
    memset(buffer, 0, 512);
    strncpy(buffer, data, 511);

    if (ata_write_sector(lba, buffer) == 0) {
        kprintf("Successfully wrote to sector %d.\n", lba);
    } else {
        kprintf("Failed to write to sector %d.\n", lba);
    }
}
   
   void ls_command() {
       bdfs_list_files();
   }
   
   void touch_command(const char* filename) {
       if (bdfs_create_file(filename) == 0) {
           kprintf("File '%s' created.\n", filename);
       } else {
           kprintf("Error creating file '%s'.\n", filename);
       }
   }
   
   void write_command(const char* filename, const char* data) {
       if (bdfs_write_file(filename, (const uint8_t*)data, strlen(data)) > 0) {
           kprintf("Wrote to file '%s'.\n", filename);
       } else {
           kprintf("Error writing to file '%s'.\n", filename);
       }
   }
   
   void cat_command(const char* filename) {
       uint8_t buffer[1024]; // 1KB buffer
       uint32_t bytes_read;
       memset(buffer, 0, 1024);
       if (bdfs_read_file(filename, buffer, &bytes_read) == 0) {
           kprintf("DEBUG: bytes_read = %d\n", bytes_read);
           // Print hex and ASCII representation
           for (uint32_t i = 0; i < bytes_read; i++) {
               kprintf("%02x ", buffer[i]);
           }
           kprintf("\n");
           for (uint32_t i = 0; i < bytes_read; i++) {
               char c = (buffer[i] >= 32 && buffer[i] <= 126) ? buffer[i] : '.';
               print_char(c, 0x07);
           }
           kprintf("\n");
       } else {
           kprintf("Error reading from file '%s'.\n", filename);
       }
   }
   
   void rm_command(const char* filename) {
       if (bdfs_delete_file(filename) == 0) {
           kprintf("File '%s' deleted.\n", filename);
       } else {
           kprintf("Error deleting file '%s'.\n", filename);
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
       kprintf("Filesystem formatted.\n");
   }
   
   void clear_command() {
    clear_screen(0x07);
}

void meminfo_command() {
    print("Physical Memory Manager (PMM) Info:\n", 0x07);
    print("  Total memory: ", 0x07);
    print_int(pmm_get_total_memory() / 1024 / 1024, 0x07);
    print(" MB\n", 0x07);
    print("  Used memory:  ", 0x07);
    print_int(pmm_get_used_memory() / 1024 / 1024, 0x07);
    print(" MB\n", 0x07);
    print("  Free memory:  ", 0x07);
    print_int(pmm_get_free_memory() / 1024 / 1024, 0x07);
    print(" MB\n", 0x07);
}

void time_command() {
    print("Uptime: ", 0x07);
    print_int(timer_ticks / 100, 0x07);
    print(" seconds\n", 0x07);
}

void halt_command() {
    print("Halting system...\n", 0x07);
    asm volatile("hlt"); // Halt the CPU
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
            print("Usage: touch <filename>\n", 0x04);
        }
    } else if (strcmp(token, "write") == 0) {
        char* filename = strtok(NULL, " ");
        char* data = strtok(NULL, ""); // The rest of the string
        if (filename && data) {
            write_command(filename, data);
        } else {
            print("Usage: write <filename> <data>\n", 0x04);
        }
    } else if (strcmp(token, "cat") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            cat_command(token);
        } else {
            print("Usage: cat <filename>\n", 0x04);
        }
    } else if (strcmp(token, "rm") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            rm_command(token);
        } else {
            print("Usage: rm <filename>\n", 0x04);
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
        print("Unknown command: ", 0x04);
        print(command, 0x04);
        print("\n", 0x04);
    }
}

void start_shell() {
    print(PROMPT, 0x07);
    while (1) {
        char c = keyboard_get_char();
        if (c == '\0') {
            continue;
        }

        if (c == '\n') {
            command_buffer[command_len] = '\0'; // Null-terminate the command
            print("\n", 0x07);
            kprintf("DEBUG: Command buffer: '%s', length: %d\n", command_buffer, command_len);
            process_command(command_buffer);
            command_len = 0;
            memset(command_buffer, 0, MAX_COMMAND_LENGTH); // Clear buffer
            print(PROMPT, 0x07);
        } else if (c == '\b') { // Backspace
            if (command_len > 0) {
                command_len--;
                print_backspace();
            }
        } else {
            if (command_len < MAX_COMMAND_LENGTH - 1) {
                command_buffer[command_len++] = c;
            }
            print_char(c, 0x07); // Echo to screen
        }
    }
}