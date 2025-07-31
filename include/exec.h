#ifndef EXEC_H
#define EXEC_H

#include "include/types.h"

// Bytecode Opcodes
#define OPCODE_SYSCALL_PRINT 0x01
#define OPCODE_SYSCALL_READ  0x02
#define OPCODE_EXIT          0x03

// Function prototypes
int execute_bdx(const char* path);
void interpret_bdx(uint8_t* bytecode);

#endif