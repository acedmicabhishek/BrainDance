#include "include/paging.h"
#include "include/regs.h"
#include "include/isr.h"
#include "include/memcore.h"

// Page directory and table placed at specific physical addresses
page_directory_t* page_directory = (page_directory_t*)0x90000;
page_table_t* first_page_table = (page_table_t*)0x91000;

// Page fault handler
void page_fault_handler(regs_t *r) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    print("\nPANIC: PAGE FAULT\n", 0x04);
    print("  Faulting Address: ", 0x07);
    print_hex(faulting_address, 0x07);
    print("\n", 0x07);
    print("  Error Code: ", 0x07);
    print_hex(r->err_code, 0x07);
    print("\n", 0x07);

    for(;;); // Halt
}

void paging_install() {
    // --- 1. Create Page Directory and Page Table ---

    // Clear the page directory and page table
    memset(page_directory, 0, sizeof(page_directory_t));
    memset(first_page_table, 0, sizeof(page_table_t));

    // Identity map the first 4MB
    for (int i = 0; i < 1024; i++) {
        uint32_t physical_addr = i * 0x1000;
        page_table_entry_t* pte = &first_page_table->pages[i];
        pte->present = 1;
        pte->rw = 1;
        pte->frame = physical_addr >> 12;
    }

    // Set the first entry in the page directory
    page_directory_entry_t* pde = &page_directory->tables[0];
    pde->present = 1;
    pde->rw = 1;
    pde->frame = (uint32_t)first_page_table >> 12;

    // --- 2. Install Page Fault Handler ---
    register_interrupt_handler(14, page_fault_handler);

    // --- 3. Enable Paging ---
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}