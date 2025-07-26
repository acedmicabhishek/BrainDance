#include "./include/idt.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void load_idt(unsigned int);  // Declared in ASM

void idt_set_gate(int n, unsigned int handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08;  // Kernel code segment
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E; // Present, ring 0, 32-bit interrupt gate
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;

    // Clear all entries
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

    // Load IDT
    load_idt((unsigned int)&idtp);
}
