extern char _bss_start;
extern char _bss_end;
extern char _stack_start;
extern char _stack_end;

#include "./include/memcore.h"
#include "./include/idt.h"
#include "./include/isr.h"

void kernel_main() {
    clear_screen(0x07);
    print("BrainDance Kernel Loaded.\n\n", 0x07);

    // Print memory diagnostics
    unsigned int bss_size   = (unsigned int)(&_bss_end - &_bss_start);
    unsigned int stack_size = (unsigned int)(&_stack_end - &_stack_start);

    print("Memory Info:\n", 0x07);
    print("  BSS Size   : ", 0x07);
    print_int(bss_size, 0x07);
    print(" bytes\n", 0x07);
    print("  Stack Size : ", 0x07);
    print_int(stack_size, 0x07);
    print(" bytes\n\n", 0x07);

    // Initialize interrupts
    idt_install();
    isr_install();
    print("INFO: IDT and ISRs installed\n", 0x02);

    // Trigger divide-by-zero to test ISR[0]
    asm volatile("int $0");
}
