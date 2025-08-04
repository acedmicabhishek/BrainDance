#include "include/shell.h"
#include "include/memcore.h"
#include "include/keyboard.h"
#include "include/pmm.h"
#include "include/timer.h"
#include "include/bdfs.h"
#include "include/ata.h"
#include "include/colors.h"
#include "include/cable.h"
#include "include/exec.h"
#include "include/calculator.h"
#include "include/ports.h"

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
    print("  reboot   - Reboot the system\n", COLOR_SYSTEM);
    print("  shutdown - Shutdown the system\n", COLOR_SYSTEM);
    print("  ls       - List files\n", COLOR_SYSTEM);
    print("  touch    - Create a file\n", COLOR_SYSTEM);
    print("  write    - Write to a file\n", COLOR_SYSTEM);
    print("  echo     - Print text to the screen\n", COLOR_SYSTEM);
    print("  cat      - Read from a file\n", COLOR_SYSTEM);
    print("  cable    - Open a text editor\n", COLOR_SYSTEM);
    print("  rm       - Remove a file\n", COLOR_SYSTEM);
    print("  mv       - Rename a file\n", COLOR_SYSTEM);
    print("  mkdir    - Create a directory\n", COLOR_SYSTEM);
    print("  cd       - Change directory\n", COLOR_SYSTEM);
    print("  sysinfo  - Display system information\n", COLOR_SYSTEM);
    print("  calc     - Evaluate a mathematical expression\n", COLOR_SYSTEM);
}

void sysinfo_command() {
    extern uint32_t _bss_start[], _kernel_end[];
    uint32_t kernel_size = (uint32_t)_kernel_end - (uint32_t)_bss_start;

    print("[sysinfo] brainDance v0.3\n", COLOR_SYSTEM);
    kprintf("[sysinfo] Kernel Size: %d KB\n", kernel_size / 1024);
    kprintf("[sysinfo] Total RAM: %d MB\n", pmm_get_total_memory() / 1024 / 1024);
    kprintf("[sysinfo] Free RAM: %d MB\n", pmm_get_free_memory() / 1024 / 1024);
    // kprintf("[sysinfo] Heap Usage: %d KB / %d KB\n", heap_get_usage() / 1024, (HEAP_END - HEAP_START) / 1024);
    kprintf("[sysinfo] Uptime: %d seconds\n", timer_ticks / 100);
    print("[sysinfo] Drivers: ATA, Keyboard, Timer\n", COLOR_SYSTEM);
    print("[sysinfo] Shell User: V\n", COLOR_SYSTEM);
}

// Simple atoi implementation
int atoi(const char* str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        res = res * 10 + str[i] - '0';
    }
    return res;
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
           print("Wrote to file '", COLOR_SUCCESS);
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
           for (uint32_t i = 0; i < bytes_read; i++) {
               print_char(buffer[i], COLOR_FILE);
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
           print(" File '", COLOR_SUCCESS);
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
           print("File '", COLOR_SUCCESS);
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
           print("Directory '", COLOR_SUCCESS);
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

void reboot_command() {
    print("Rebooting system...\n", COLOR_SYSTEM);
    // Using the keyboard controller to reset the system
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
}

void shutdown_command() {
    print("Shutting down system...\n", COLOR_SYSTEM);
    outw(0x604, 0x2000); // QEMU specific shutdown
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
    } else if (strcmp(token, "reboot") == 0) {
        reboot_command();
    } else if (strcmp(token, "shutdown") == 0) {
        shutdown_command();
    } else if (strcmp(token, "sysinfo") == 0) {
        sysinfo_command();
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
    } else if (strcmp(token, "cable") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            cable_main(token);
            clear_screen(COLOR_SYSTEM);
        } else {
            print("Usage: cable <filename>\n", COLOR_ERROR);
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
    } else if (strcmp(token, "calc") == 0) {
       char* expression = strtok(NULL, "");
       if (expression) {
           calculator_main(expression);
       } else {
           print("Usage: calc <expression>\n", COLOR_ERROR);
       }
   } else if (strlen(command) > 0) {
       if (ends_with(command, ".bdx")) {
           if (execute_bdx(command) != 0) {
               print("Error executing ", COLOR_ERROR);
               print(command, COLOR_ERROR);
               print("\n", COLOR_ERROR);
           }
       } else {
           print("Unknown command: ", COLOR_ERROR);
           print(command, COLOR_ERROR);
           print("\n", COLOR_ERROR);
       }
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