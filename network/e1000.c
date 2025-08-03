#include "../include/e1000.h"
#include "../include/pci.h"
#include "../include/paging.h"
#include "../include/memcore.h"

static volatile uint32_t* e1000_regs = 0;

bool e1000_init(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t bar0_raw = pci_config_read(bus, dev, func, 0x10);
    if (bar0_raw & 1) return false; // Not MMIO

    uint32_t mmio_base = bar0_raw & 0xFFFFFFF0;
    
    // Map the MMIO space
    map_page(mmio_base, mmio_base, PTE_PRESENT | PTE_RW);
    e1000_regs = (volatile uint32_t*) mmio_base;

    print("E1000 MMIO Base: ", 0x07);
    print_hex(mmio_base, 0x07);
    print("\n", 0x07);

    // Read status register to verify
    uint32_t status = e1000_regs[2]; // STATUS is at offset 0x8, which is index 2 of a uint32_t pointer
    print("E1000 Status: ", 0x07);
    print_hex(status, 0x07);
    print("\n", 0x07);

    return true;
}