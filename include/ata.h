#ifndef ATA_H
#define ATA_H

#include "pmm.h"

// ATA I/O Ports
#define ATA_DATA_PORT         0x1F0
#define ATA_ERROR_PORT        0x1F1
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT      0x1F3
#define ATA_LBA_MID_PORT      0x1F4
#define ATA_LBA_HIGH_PORT     0x1F5
#define ATA_DRIVE_HEAD_PORT   0x1F6
#define ATA_STATUS_CMD_PORT   0x1F7
#define ATA_CONTROL_PORT      0x3F6

// ATA Commands
#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

// ATA Status Bits
#define ATA_SR_BSY  0x80    // Busy
#define ATA_SR_DRDY 0x40    // Drive ready
#define ATA_SR_DF   0x20    // Drive write fault
#define ATA_SR_DSC  0x10    // Drive seek complete
#define ATA_SR_DRQ  0x08    // Data request ready
#define ATA_SR_CORR 0x04    // Corrected data
#define ATA_SR_IDX  0x02    // Index
#define ATA_SR_ERR  0x01    // Error

void ata_init();
int  ata_read_sector(uint32_t lba, void* buffer);
int  ata_write_sector(uint32_t lba, const void* buffer);
int  ata_status();

#endif // ATA_H