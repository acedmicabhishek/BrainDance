#include "include/shell.h"
#include "include/memcore.h"
#include "include/keyboard.h"
#include "include/pmm.h"
#include "include/timer.h"

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
    print_int(timer_ticks / 100, 0x07); // Assuming 100 ticks per second
    print(" seconds\n", 0x07);
}

void halt_command() {
    print("Halting system...\n", 0x07);
    asm volatile("hlt"); // Halt the CPU
}

// Function to process a command
void process_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        help_command();
    } else if (strcmp(command, "clear") == 0) {
        clear_command();
    } else if (strcmp(command, "meminfo") == 0) {
        meminfo_command();
    } else if (strcmp(command, "time") == 0) {
        time_command();
    } else if (strcmp(command, "halt") == 0) {
        halt_command();
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
                print_char(c, 0x07);
            }
        }
    }
}