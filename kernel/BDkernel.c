extern char _bss_start;
extern char _bss_end;
extern char _stack_start;
extern char _stack_end;

#include "include/memcore.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/irq.h"
#include "include/timer.h"
#include "include/keyboard.h"
#include "include/pmm.h"
#include "include/paging.h"
#include "include/shell.h"
#include "include/heap.h"
#include "include/bdfs.h"
#include "include/ata.h"
#include "include/pci.h"
#include "include/cpu.h"

#define RAMDISK_BASE 0x200000
void kernel_main() {
    uint32_t mmap_entries = *(uint32_t*)0x900;
    uint32_t mmap_addr = 0x1000;

    pmm_init(mmap_addr, mmap_entries);

    clear_screen(0x07);
    print("BrainDance Kernel Loaded.\n\n", 0x04);

    unsigned int bss_size   = (unsigned int)(&_bss_end - &_bss_start);
    unsigned int stack_size = (unsigned int)(&_stack_end - &_stack_start);

    print("Memory Info:\n", 0x07);
    print("  BSS Size   : ", 0x07);
    print_int(bss_size, 0x07);
    print(" bytes\n", 0x07);
    print("  Stack Size : ", 0x07);
    print_int(stack_size, 0x07);
    print(" bytes\n\n", 0x07);

    idt_install();
    isr_install();
    irq_install();
    print("INFO: IDT, ISRs, and IRQs installed\n", 0x02);

    paging_install();
    print("INFO: Paging enabled\n", 0x02);

    print("INFO: Kernel Heap initialized\n", 0x02);

    timer_install();
    print("INFO: Timer installed\n", 0x02);

    keyboard_install();
    print("INFO: Keyboard installed\n", 0x02);

    ata_init();

    bdfs_init();
    print("INFO: BDFS initialized\n", 0x02);

    asm volatile ("sti");

    pci_scan_all();

    cpu_init();

    start_shell();

    for(;;);
}
