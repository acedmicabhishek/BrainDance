#include "../include/pci.h"
#include "../include/ports.h"
#include "../include/types.h"
#include "../include/memcore.h"
#include "../include/e1000.h"


uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (1U << 31)              
                     | ((uint32_t)bus << 16)
                     | ((uint32_t)device << 11)
                     | ((uint32_t)func << 8)
                     | (offset & 0xFC);        

    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_scan_all() {
    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t dev = 0; dev < 32; ++dev) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint32_t vendor_device = pci_config_read(bus, dev, func, 0x00);
                uint16_t vendor_id = vendor_device & 0xFFFF;

                if (vendor_id == 0xFFFF) continue; 

                uint32_t class_code = pci_config_read(bus, dev, func, 0x08);
                uint8_t class_id = (class_code >> 24) & 0xFF;
                uint8_t subclass = (class_code >> 16) & 0xFF;

                
                if (class_id == 0x02 && subclass == 0x00) {
                    print("PCI Ethernet @ Bus ", 0x07);
                    print_int(bus, 0x07);
                    print(", Device ", 0x07);
                    print_int(dev, 0x07);
                    print(", Func ", 0x07);
                    print_int(func, 0x07);
                    print("\n", 0x07);
                    e1000_init(bus, dev, func);
                }
            }
        }
    }
}