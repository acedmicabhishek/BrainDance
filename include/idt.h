#pragma once

#include "memcore.h"

#define IDT_ENTRIES 256

struct idt_entry {
    unsigned short offset_low;   // Lower 16 bits of handler
    unsigned short selector;     // Kernel code segment selector
    unsigned char zero;          // Always zero
    unsigned char type_attr;     // Type and attributes
    unsigned short offset_high;  // Higher 16 bits of handler
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void idt_set_gate(int n, unsigned int handler);
void idt_install();
