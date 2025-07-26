#include "include/memcore.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/pmm.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void isr_handler(regs_t *r) {
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    } else {
        print("CPU Exception [", 0x04);
        print_int(r->int_no, 0x04);
        print("] occurred. Halting...\n", 0x04);
        for (;;);
    }
}

void isr_install() {
    // Clear all interrupt handlers
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = 0;
    }

    extern void isr0(), isr1(), isr2(), isr3(), isr4(), isr5(), isr6(), isr7();
    extern void isr8(), isr9(), isr10(), isr11(), isr12(), isr13(), isr14();
    extern void isr15(), isr16(), isr17(), isr18(), isr19(), isr20(), isr21();
    extern void isr22(), isr23(), isr24(), isr25(), isr26(), isr27(), isr28();
    extern void isr29(), isr30(), isr31();

    idt_set_gate(0, (unsigned)isr0);
    idt_set_gate(1, (unsigned)isr1);
    idt_set_gate(2, (unsigned)isr2);
    idt_set_gate(3, (unsigned)isr3);
    idt_set_gate(4, (unsigned)isr4);
    idt_set_gate(5, (unsigned)isr5);
    idt_set_gate(6, (unsigned)isr6);
    idt_set_gate(7, (unsigned)isr7);
    idt_set_gate(8, (unsigned)isr8);
    idt_set_gate(9, (unsigned)isr9);
    idt_set_gate(10, (unsigned)isr10);
    idt_set_gate(11, (unsigned)isr11);
    idt_set_gate(12, (unsigned)isr12);
    idt_set_gate(13, (unsigned)isr13);
    idt_set_gate(14, (unsigned)isr14);
    idt_set_gate(15, (unsigned)isr15);
    idt_set_gate(16, (unsigned)isr16);
    idt_set_gate(17, (unsigned)isr17);
    idt_set_gate(18, (unsigned)isr18);
    idt_set_gate(19, (unsigned)isr19);
    idt_set_gate(20, (unsigned)isr20);
    idt_set_gate(21, (unsigned)isr21);
    idt_set_gate(22, (unsigned)isr22);
    idt_set_gate(23, (unsigned)isr23);
    idt_set_gate(24, (unsigned)isr24);
    idt_set_gate(25, (unsigned)isr25);
    idt_set_gate(26, (unsigned)isr26);
    idt_set_gate(27, (unsigned)isr27);
    idt_set_gate(28, (unsigned)isr28);
    idt_set_gate(29, (unsigned)isr29);
    idt_set_gate(30, (unsigned)isr30);
    idt_set_gate(31, (unsigned)isr31);
}
