#pragma once

#include "memcore.h"

#define IDT_ENTRIES 256

struct idt_entry {
    unsigned short offset_low;   
    unsigned short selector;     
    unsigned char zero;          
    unsigned char type_attr;     
    unsigned short offset_high;  
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

void idt_set_gate(int n, unsigned int handler);
void idt_install();
