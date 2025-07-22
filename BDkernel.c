#include "./lib/memcore.h"

void kernel_main() {
    print(">> BrainDance OS - A CyberNet Project", 0x0D); // Bright magenta
    print(">> Version: 0.1", 0x0E);                      // Bright yellow
    print(">> Kernel Booted Successfully", 0x0B);        // Bright cyan

    while (1) {}
}
