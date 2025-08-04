#include "include/pic.h"
#include "include/ports.h"

#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

void pic_remap(int offset1, int offset2) {
    
    unsigned char a1 = inb(PIC1_DATA);
    unsigned char a2 = inb(PIC2_DATA);

    
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    
    outb(PIC1_DATA, 4);
    
    outb(PIC2_DATA, 2);

    
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}