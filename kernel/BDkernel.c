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
#include "include/vesa.h"

// Define the RAM disk base address
#define RAMDISK_BASE 0x200000

void kernel_main() {
    // Read memory map info from the bootloader-populated addresses
    uint32_t mmap_entries = *(uint32_t*)0x900;
    uint32_t mmap_addr = 0x1000;

    // Initialize the PMM
    pmm_init(mmap_addr, mmap_entries);

    // Initialize paging
    paging_install();
    print("INFO: Paging enabled\n", 0x02);

    // Hardcoded values for 1920x1080x16bpp
    uint32_t fb_addr = 0xE0000000;
    uint16_t width = 1920;
    uint16_t height = 1080;
    uint16_t pitch = 3840;
    uint32_t fb_size = pitch * height;

    // Map VESA framebuffer
    map_range(fb_addr, 0xC0000000, fb_size, PTE_PRESENT | PTE_RW | PTE_USER);

    // Initialize VESA with the mapped address and hardcoded dimensions
    vesa_init((uint16_t*)0xC0000000, width, height, pitch);

    clear_screen(0x01); // Clear with blue
    print("BrainDance Kernel Loaded.\n\n", 0x0F);

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
    irq_install();
    print("INFO: IDT, ISRs, and IRQs installed\n", 0x02);

    // Heap is initialized by global heap_ptr in memory/heap.c
    // print("INFO: Kernel Heap initialized\n", 0x02);

    // Initialize timer
    timer_install();
    print("INFO: Timer installed\n", 0x02);

    // Initialize keyboard
    keyboard_install();
    print("INFO: Keyboard installed\n", 0x02);

    // Initialize ATA driver
    ata_init();

    // Initialize BDFS
    bdfs_init();
    print("INFO: BDFS initialized\n", 0x02);

    // Enable interrupts
    asm volatile ("sti");

    // Start the shell
    start_shell();

    // Wait for interrupts (should not be reached if shell loops indefinitely)
    for(;;);
}
