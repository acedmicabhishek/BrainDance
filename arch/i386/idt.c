#include "./include/idt.h"

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void load_idt(unsigned int);  

void idt_set_gate(int n, unsigned int handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08;  
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E; 
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;

    
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);

    
    load_idt((unsigned int)&idtp);
}
