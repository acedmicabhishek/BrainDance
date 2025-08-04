#ifndef ATA_H
#define ATA_H

#include "pmm.h"


#define ATA_DATA_PORT         0x1F0
#define ATA_ERROR_PORT        0x1F1
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT      0x1F3
#define ATA_LBA_MID_PORT      0x1F4
#define ATA_LBA_HIGH_PORT     0x1F5
#define ATA_DRIVE_HEAD_PORT   0x1F6
#define ATA_STATUS_CMD_PORT   0x1F7
#define ATA_CONTROL_PORT      0x3F6


#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC


#define ATA_SR_BSY  0x80    
#define ATA_SR_DRDY 0x40    
#define ATA_SR_DF   0x20    
#define ATA_SR_DSC  0x10    
#define ATA_SR_DRQ  0x08    
#define ATA_SR_CORR 0x04    
#define ATA_SR_IDX  0x02    
#define ATA_SR_ERR  0x01    

void ata_init();
int  ata_read_sector(uint32_t lba, void* buffer);
int  ata_write_sector(uint32_t lba, const void* buffer);
int  ata_status();

#endif 