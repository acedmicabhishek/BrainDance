#pragma once

#include <include/types.h>

uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pci_scan_all();
void pci_list_devices();