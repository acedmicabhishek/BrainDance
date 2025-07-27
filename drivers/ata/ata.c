#include "../../include/ata.h"
#include "../../include/ports.h"
#include "../../include/memcore.h"

// 400ns delay
static void ata_delay() {
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
}

int ata_status() {
    return inb(ATA_STATUS_CMD_PORT);
}

// Wait for the drive to be ready
static int ata_wait_ready() {
    while (inb(ATA_STATUS_CMD_PORT) & ATA_SR_BSY);
    return 0;
}

void ata_init() {
    // Select the master drive on the primary bus
    outb(ATA_DRIVE_HEAD_PORT, 0xE0);
    ata_delay();

    // Check if the drive is present
    if (ata_status() == 0xFF) {
        kprintf("ATA: No drive found\n");
        return;
    }

    kprintf("ATA: Drive found, waiting for ready...\n");
    ata_wait_ready();
    kprintf("ATA: Drive ready\n");
}

int ata_read_sector(uint32_t lba, void* buffer) {
    ata_wait_ready();

    // Send the command to read sectors
    outb(ATA_DRIVE_HEAD_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR_PORT, 0x00);
    outb(ATA_SECTOR_COUNT_PORT, 1);
    outb(ATA_LBA_LOW_PORT, (uint8_t)lba);
    outb(ATA_LBA_MID_PORT, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH_PORT, (uint8_t)(lba >> 16));
    outb(ATA_STATUS_CMD_PORT, ATA_CMD_READ_SECTORS);

    ata_wait_ready();

    // Check for errors
    uint8_t status = ata_status();
    if (status & ATA_SR_ERR) {
        kprintf("ATA: Read error\n");
        return -1;
    }

    // Read the data from the data port
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inb(ATA_DATA_PORT) | (inb(ATA_DATA_PORT) << 8);
    }

    kprintf("ATA: Read sector OK\n");
    return 0;
}

int ata_write_sector(uint32_t lba, const void* buffer) {
    ata_wait_ready();

    // Send the command to write sectors
    outb(ATA_DRIVE_HEAD_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR_PORT, 0x00);
    outb(ATA_SECTOR_COUNT_PORT, 1);
    outb(ATA_LBA_LOW_PORT, (uint8_t)lba);
    outb(ATA_LBA_MID_PORT, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH_PORT, (uint8_t)(lba >> 16));
    outb(ATA_STATUS_CMD_PORT, ATA_CMD_WRITE_SECTORS);

    ata_wait_ready();

    // Check for errors
    uint8_t status = ata_status();
    if (status & ATA_SR_ERR) {
        kprintf("ATA: Write error\n");
        return -1;
    }

    // Write the data to the data port
    const uint16_t* ptr = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outb(ATA_DATA_PORT, ptr[i] & 0xFF);
        outb(ATA_DATA_PORT, (ptr[i] >> 8) & 0xFF);
    }

    // Flush the cache
    outb(ATA_STATUS_CMD_PORT, 0xE7);
    ata_wait_ready();

    kprintf("ATA: Write sector OK\n");
    return 0;
}