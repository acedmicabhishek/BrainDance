#include "./lib/memcore.h"

void kernel_main() {
    print(">> BrainDance OS - A CyberNet Project\n", 0x0D); // Bright magenta
    print(">> Version: 0.1\n", 0x0E);                      // Bright yellow
    print(">> Kernel Booted Successfully\n", 0x0B);        // Bright cyan
    
    while (1) {}
}
