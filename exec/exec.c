#include "include/exec.h"
#include "include/bdfs.h"
#include "include/memcore.h"
#include "include/colors.h"

#define MAX_BYTECODE_SIZE 1024

void interpret_bdx(uint8_t* bytecode) {
    int ip = 0; // Instruction Pointer
    while (1) {
        uint8_t opcode = bytecode[ip++];
        switch (opcode) {
            case OPCODE_SYSCALL_PRINT: {
                char* str = (char*)&bytecode[ip];
                print(str, COLOR_INPUT);
                ip += strlen(str) + 1; // +1 for null terminator
                break;
            }
            case OPCODE_EXIT: {
                return;
            }
            default:
                // Handle unknown opcode
                return;
        }
    }
}

int execute_bdx(const char* path) {
    uint8_t bytecode[MAX_BYTECODE_SIZE];
    uint32_t bytes_read;

    if (bdfs_read_file(path, bytecode, &bytes_read) != 0) {
        return -1; // File not found or error reading
    }

    interpret_bdx(bytecode);
    return 0;
}