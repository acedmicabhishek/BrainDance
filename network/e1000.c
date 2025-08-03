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
#define RCTL_SZ_2048 (0 << 16)
#define RCTL_MPE     (1 << 4)

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

static uint32_t e1000_read(uint16_t offset) {
    return e1000_regs[offset / 4];
}

static void dump_packet(uint8_t* buf, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) {
            print("0", 0x07);
        }
        print_hex(buf[i], 0x07);
        print(" ", 0x07);
        if ((i + 1) % 16 == 0) {
            print("\n", 0x07);
        }
    }
    print("\n", 0x07);
}

void e1000_rx_init() {
    rx_ring = alloc_aligned(RX_DESC_COUNT * sizeof(struct e1000_rx_desc), 16);
    
    for (int i = 0; i < RX_DESC_COUNT; ++i) {
        uintptr_t phys = (uintptr_t)pmm_alloc_block();
        rx_buffers[i] = kmalloc(2048);
        map_page(phys, (uint32_t)rx_buffers[i], PTE_PRESENT | PTE_RW);
        rx_ring[i].addr = (uint64_t)phys;
        rx_ring[i].status = 0;
    }

    uintptr_t phys_ring = get_phys_addr((uintptr_t)rx_ring);
    e1000_write(E1000_RDBAL, (uint32_t)phys_ring);
    e1000_write(E1000_RDBAH, 0);
    e1000_write(E1000_RDLEN, RX_DESC_COUNT * sizeof(struct e1000_rx_desc));
    e1000_write(E1000_RDH, 0);
    e1000_write(E1000_RDT, RX_DESC_COUNT - 1);

    uint32_t rctl = RCTL_EN | RCTL_BAM | RCTL_SECRC | RCTL_SZ_2048 | RCTL_MPE;
    e1000_write(E1000_RCTL, rctl);
}

void e1000_tx_init() {
    tx_ring = alloc_aligned(TX_DESC_COUNT * sizeof(struct e1000_tx_desc), 16);

    for (int i = 0; i < TX_DESC_COUNT; ++i) {
        uintptr_t phys = (uintptr_t)pmm_alloc_block();
        tx_buffers[i] = kmalloc(2048);
        map_page(phys, (uint32_t)tx_buffers[i], PTE_PRESENT | PTE_RW);
        tx_ring[i].addr = (uint64_t)phys;
        tx_ring[i].status = 0;
    }

    uintptr_t phys_ring = get_phys_addr((uintptr_t)tx_ring);
    e1000_write(E1000_TDBAL, (uint32_t)phys_ring);
    e1000_write(E1000_TDBAH, 0);
    e1000_write(E1000_TDLEN, TX_DESC_COUNT * sizeof(struct e1000_tx_desc));
    e1000_write(E1000_TDH, 0);
    e1000_write(E1000_TDT, 0);

    uint32_t tctl = TCTL_EN | TCTL_PSP;
    e1000_write(E1000_TCTL, tctl);
}

bool e1000_init(uint8_t bus, uint8_t dev, uint8_t func) {
    uint32_t bar0_raw = pci_config_read(bus, dev, func, 0x10);
    if (bar0_raw & 1) return false;

    // Enable bus mastering
    uint32_t pci_command = pci_config_read(bus, dev, func, 0x04);
    pci_command |= (1 << 2);
    pci_config_write(bus, dev, func, 0x04, pci_command);

    uint32_t mmio_base = bar0_raw & 0xFFFFFFF0;
    // Map a larger region for MMIO
    for (uint32_t i = 0; i < 0x10000; i += 0x1000) {
        map_page(mmio_base + i, mmio_base + i, PTE_PRESENT | PTE_RW);
    }
    e1000_regs = (volatile uint32_t*)mmio_base;

    print("E1000 MMIO Base: ", 0x07);
    print_hex(mmio_base, 0x07);
    print("\n", 0x07);

    // Reset the device
    e1000_write(0x0000, e1000_read(0x0000) | (1 << 26));
    while(e1000_read(0x0000) & (1 << 26));

    // Clear multicast table
    for(int i = 0; i < 128; i++) {
        e1000_write(0x5200 + (i * 4), 0);
    }

    // Set link up
    e1000_write(0x0000, e1000_read(0x0000) | (1 << 6));

    // Disable interrupts
    e1000_write(0x00D8, 0xFFFFFFFF);

    // Wait for link
    print("Waiting for link...\n", 0x07);
    while(!(e1000_read(0x08) & 2));
    print("Link up.\n", 0x07);

    uint32_t status = e1000_read(0x08);
    print("E1000 Status: ", 0x07);
    print_hex(status, 0x07);
    print("\n", 0x07);

    e1000_rx_init();
    e1000_tx_init();

    return true;
}

void e1000_poll_rx() {
    if (!e1000_regs) {
        print("E1000 not initialized.\n", 0x04);
        return;
    }

    static int idx = 0;
    struct e1000_rx_desc* desc = &rx_ring[idx];

    uint32_t rdh = e1000_read(E1000_RDH);
    uint32_t rdt = e1000_read(E1000_RDT);
    kprintf("RDH: %d, RDT: %d, idx: %d, status: %x\n", rdh, rdt, idx, desc->status);

    while (desc->status & 0x01) {
        kprintf("Packet received at index %d\n", idx);
        dump_packet(rx_buffers[idx], desc->length);
        desc->status = 0;
        
        e1000_write(E1000_RDT, idx);
        
        idx = (idx + 1) % RX_DESC_COUNT;
        desc = &rx_ring[idx];
    }
}