#include "../../include/ata.h"
#include "../../include/ports.h"
#include "../../include/memcore.h"


static void ata_delay() {
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
    inb(ATA_STATUS_CMD_PORT);
}

int ata_status() {
    return inb(ATA_STATUS_CMD_PORT);
}



static uint8_t ata_poll() {
    while(inb(ATA_STATUS_CMD_PORT) & ATA_SR_BSY);
    return inb(ATA_STATUS_CMD_PORT);
}

void ata_init() {
    
    outb(ATA_DRIVE_HEAD_PORT, 0xE0);
    ata_delay();

    
    if (ata_status() == 0xFF) {
        kprintf("ATA: No drive found\n");
        return;
    }

    kprintf("ATA: Drive found, waiting for ready...\n");
    ata_poll();
    kprintf("ATA: Drive ready\n");
}

int ata_read_sector(uint32_t lba, void* buffer) {
    uint8_t status;

    
    status = ata_poll();
    if ((status & ATA_SR_DRDY) == 0) {
        kprintf("ATA: Drive not ready for read command\n");
        return -1;
    }

    
    outb(ATA_DRIVE_HEAD_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR_PORT, 0x00);
    outb(ATA_SECTOR_COUNT_PORT, 1);
    outb(ATA_LBA_LOW_PORT, (uint8_t)lba);
    outb(ATA_LBA_MID_PORT, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH_PORT, (uint8_t)(lba >> 16));
    outb(ATA_STATUS_CMD_PORT, ATA_CMD_READ_SECTORS);

    
    status = ata_poll();

    
    if (status & ATA_SR_ERR) {
        kprintf("ATA: Read error\n");
        return -1;
    }

    if (!(status & ATA_SR_DRQ)) {
        kprintf("ATA: DRQ not set after read\n");
        return -1;
    }

    
    uint16_t* ptr = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(ATA_DATA_PORT);
    }

    kprintf("ATA: Read sector OK\n");
    return 0;
}

int ata_write_sector(uint32_t lba, const void* buffer) {
    uint8_t status;

    
    status = ata_poll();
    if ((status & ATA_SR_DRDY) == 0) {
        kprintf("ATA: Drive not ready for write command\n");
        return -1;
    }

    
    outb(ATA_DRIVE_HEAD_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_ERROR_PORT, 0x00);
    outb(ATA_SECTOR_COUNT_PORT, 1);
    outb(ATA_LBA_LOW_PORT, (uint8_t)lba);
    outb(ATA_LBA_MID_PORT, (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH_PORT, (uint8_t)(lba >> 16));
    outb(ATA_STATUS_CMD_PORT, ATA_CMD_WRITE_SECTORS);

    
    status = ata_poll();

    
    if (status & ATA_SR_ERR) {
        kprintf("ATA: Write error\n");
        return -1;
    }

    if (!(status & ATA_SR_DRQ)) {
        kprintf("ATA: DRQ not set after write\n");
        return -1;
    }

    
    const uint16_t* ptr = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA_PORT, ptr[i]);
    }

    
    outb(ATA_STATUS_CMD_PORT, 0xE7);
    ata_poll();

    kprintf("ATA: Write sector OK\n");
    return 0;
}