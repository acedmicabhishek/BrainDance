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

    // Add recursive mapping (map the last PDE to the page directory itself)
    page_directory->tables[1023].present = 1;
    page_directory->tables[1023].rw = 1;
    page_directory->tables[1023].frame = (uint32_t)page_directory >> 12;

    // --- 2. Install Page Fault Handler ---
    register_interrupt_handler(14, page_fault_handler);

    // --- 3. Enable Paging ---
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void map_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t flags) {
    // Calculate page directory and page table indices
    uint32_t pd_idx = virt_addr >> 22;
    uint32_t pt_idx = (virt_addr >> 12) & 0x03FF;

    // Get the page directory entry
    page_directory_entry_t* pde = &page_directory->tables[pd_idx];

    // If the page directory entry is not present, create a new page table
    if (!pde->present) {
        // Allocate a new page table from physical memory
        uint32_t new_pt_phys_addr = pmm_alloc_block();
        if (new_pt_phys_addr == 0) {
            // Handle allocation failure (e.g., out of memory)
            print("PANIC: Out of physical memory for page table\n", 0x04);
            for(;;);
        }

        // Map the new page table into the page directory
        pde->present = 1;
        pde->rw = 1; // Read/Write
        pde->user = 1; // User-mode access
        pde->frame = new_pt_phys_addr >> 12;


        page_table_t* new_pt_virt = (page_table_t*)(0xFFC00000 | (pd_idx << 12));
        memset(new_pt_virt, 0, sizeof(page_table_t));
    }

    // Get the page table using the recursive mapping
    page_table_t* page_table = (page_table_t*)(0xFFC00000 | (pd_idx << 12));

    // Get the page table entry
    page_table_entry_t* pte = &page_table->pages[pt_idx];

    // Set the page table entry
    pte->present = (flags & PTE_PRESENT) ? 1 : 0;
    pte->rw = (flags & PTE_RW) ? 1 : 0;
    pte->user = (flags & PTE_USER) ? 1 : 0;
    pte->frame = phys_addr >> 12;

    // Invalidate TLB for the virtual address
    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

void unmap_page(uint32_t virt_addr) {
    uint32_t pd_idx = virt_addr >> 22;
    uint32_t pt_idx = (virt_addr >> 12) & 0x03FF;

    page_directory_entry_t* pde = &page_directory->tables[pd_idx];

    if (!pde->present) {
        // Page directory entry not present, so page is not mapped
        return;
    }

    page_table_t* page_table = (page_table_t*)(pde->frame << 12);
    page_table_entry_t* pte = &page_table->pages[pt_idx];

    // Clear the page table entry
    memset(pte, 0, sizeof(page_table_entry_t));

    // Invalidate TLB for the virtual address
    asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");

}

uint32_t get_phys_addr(uint32_t virt_addr) {
    uint32_t pd_idx = virt_addr >> 22;
    uint32_t pt_idx = (virt_addr >> 12) & 0x03FF;

    page_directory_entry_t* pde = &page_directory->tables[pd_idx];

    if (!pde->present) {
        return 0; // Not mapped
    }

    page_table_t* page_table = (page_table_t*)(0xFFC00000 | (pd_idx << 12));
    page_table_entry_t* pte = &page_table->pages[pt_idx];

    if (!pte->present) {
        return 0; // Not mapped
    }

    // Calculate physical address
    return (pte->frame << 12) + (virt_addr & 0xFFF);
}