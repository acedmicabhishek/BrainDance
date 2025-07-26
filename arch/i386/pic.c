#include "include/pic.h"
#include "include/ports.h"

#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

void pic_remap(int offset1, int offset2) {
    // Save masks
    unsigned char a1 = inb(PIC1_DATA);
    unsigned char a2 = inb(PIC2_DATA);

    // Starts the initialization sequence (in cascade mode)
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    // Set new vector offsets
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    // Tell master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);
    // Tell slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);

    // Set 8086/88 (MCS-80) mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Restore saved masks
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void pic_send_eoi(unsigned char irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}