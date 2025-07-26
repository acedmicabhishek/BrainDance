#include "include/irq.h"
#include "include/idt.h"
#include "include/pic.h"
#include "include/ports.h"

extern void* irq_routines[16];

void *irq_routines[16] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irq_install_handler(int irq, void (*handler)(struct regs *r)) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

void irq_handler(struct regs *r) {
    void (*handler)(struct regs *r);

    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    pic_send_eoi(r->int_no - 32);
}

extern void irq0(), irq1(), irq2(), irq3(), irq4(), irq5(), irq6(), irq7(),
             irq8(), irq9(), irq10(), irq11(), irq12(), irq13(), irq14(), irq15();

void irq_install() {
    pic_remap(0x20, 0x28);

    idt_set_gate(32, (unsigned)irq0);
    idt_set_gate(33, (unsigned)irq1);
    idt_set_gate(34, (unsigned)irq2);
    idt_set_gate(35, (unsigned)irq3);
    idt_set_gate(36, (unsigned)irq4);
    idt_set_gate(37, (unsigned)irq5);
    idt_set_gate(38, (unsigned)irq6);
    idt_set_gate(39, (unsigned)irq7);
    idt_set_gate(40, (unsigned)irq8);
    idt_set_gate(41, (unsigned)irq9);
    idt_set_gate(42, (unsigned)irq10);
    idt_set_gate(43, (unsigned)irq11);
    idt_set_gate(44, (unsigned)irq12);
    idt_set_gate(45, (unsigned)irq13);
    idt_set_gate(46, (unsigned)irq14);
    idt_set_gate(47, (unsigned)irq15);

    // Enable all IRQs
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}