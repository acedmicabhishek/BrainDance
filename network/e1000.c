#include "../include/e1000.h"
#include "../include/pci.h"
#include "../include/paging.h"
#include "../include/memcore.h"
#include "../include/heap.h"
#include "../include/pmm.h"

#define E1000_RDBAL 0x2800
#define E1000_RDBAH 0x2804
#define E1000_RDLEN 0x2808
#define E1000_RDH   0x2810
#define E1000_RDT   0x2818
#define E1000_RCTL  0x0100

#define RCTL_EN      (1 << 1)
#define RCTL_BAM     (1 << 15)
#define RCTL_SECRC   (1 << 26)

#define E1000_TDBAL 0x3800
#define E1000_TDBAH 0x3804
#define E1000_TDLEN 0x3808
#define E1000_TDH   0x3810
#define E1000_TDT   0x3818
#define E1000_TCTL  0x0400

#define TCTL_EN     (1 << 1)
#define TCTL_PSP    (1 << 3)

static volatile uint32_t* e1000_regs = 0;
static struct e1000_rx_desc* rx_ring;
static uint8_t* rx_buffers[RX_DESC_COUNT];
static struct e1000_tx_desc* tx_ring;
static uint8_t* tx_buffers[TX_DESC_COUNT];

static void e1000_write(uint16_t offset, uint32_t value) {
    e1000_regs[offset / 4] = value;
}

bool e1000_init(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t bar0_raw = pci_config_read(bus, dev, func, 0x10);
    if (bar0_raw & 1) return false;

    uint32_t mmio_base = bar0_raw & 0xFFFFFFF0;
    // Map a larger region for MMIO
    for (uint32_t i = 0; i < 0x10000; i += 0x1000) {
        map_page(mmio_base + i, mmio_base + i, PTE_PRESENT | PTE_RW);
    }
    e1000_regs = (volatile uint32_t*)mmio_base;

    print("E1000 MMIO Base: ", 0x07);
    print_hex(mmio_base, 0x07);
    print("\n", 0x07);

    uint32_t status = e1000_regs[2];
    print("E1000 Status: ", 0x07);
    print_hex(status, 0x07);
    print("\n", 0x07);

    // RX Ring Setup
    rx_ring = alloc_aligned(sizeof(struct e1000_rx_desc) * RX_DESC_COUNT, 16);
    for (int i = 0; i < RX_DESC_COUNT; ++i) {
        void* phys_addr = pmm_alloc_block();
        rx_buffers[i] = kmalloc(4096); // Allocate virtual space
        map_page((uint32_t)phys_addr, (uint32_t)rx_buffers[i], PTE_PRESENT | PTE_RW);
        rx_ring[i].addr = (uint64_t)phys_addr;
        rx_ring[i].status = 0;
    }

    e1000_write(E1000_RDBAL, (uint32_t)(uint64_t)get_phys_addr((uint32_t)rx_ring));
    e1000_write(E1000_RDBAH, (uint32_t)((uint64_t)get_phys_addr((uint32_t)rx_ring) >> 32));
    e1000_write(E1000_RDLEN, RX_DESC_COUNT * sizeof(struct e1000_rx_desc));
    e1000_write(E1000_RDH, 0);
    e1000_write(E1000_RDT, RX_DESC_COUNT - 1);
    uint32_t rctl = RCTL_EN | RCTL_BAM | RCTL_SECRC;
    e1000_write(E1000_RCTL, rctl);

    // TX Ring Setup
    tx_ring = alloc_aligned(sizeof(struct e1000_tx_desc) * TX_DESC_COUNT, 16);
    for (int i = 0; i < TX_DESC_COUNT; ++i) {
        void* phys_addr = pmm_alloc_block();
        tx_buffers[i] = kmalloc(4096); // Allocate virtual space
        map_page((uint32_t)phys_addr, (uint32_t)tx_buffers[i], PTE_PRESENT | PTE_RW);
        tx_ring[i].addr = (uint64_t)phys_addr;
        tx_ring[i].cmd = 0;
    }

    e1000_write(E1000_TDBAL, (uint32_t)(uint64_t)get_phys_addr((uint32_t)tx_ring));
    e1000_write(E1000_TDBAH, (uint32_t)((uint64_t)get_phys_addr((uint32_t)tx_ring) >> 32));
    e1000_write(E1000_TDLEN, TX_DESC_COUNT * sizeof(struct e1000_tx_desc));
    e1000_write(E1000_TDH, 0);
    e1000_write(E1000_TDT, 0);
    uint32_t tctl = TCTL_EN | TCTL_PSP;
    e1000_write(E1000_TCTL, tctl);
    print("TX ring enabled.\n", 0x07);

    return true;
}