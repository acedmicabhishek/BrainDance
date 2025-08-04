#include "include/timer.h"
#include "include/irq.h"
#include "include/ports.h"
#include "include/memcore.h"
#include "include/cpu.h"

unsigned int timer_ticks = 0;

void timer_handler(struct regs *r) {
    timer_ticks++;
    cpu_tick();
}

void timer_install() {
    unsigned int divisor = 1193182 / 100;

    outb(0x43, 0x36);

    unsigned char l = (unsigned char)(divisor & 0xFF);
    unsigned char h = (unsigned char)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);

    irq_install_handler(0, timer_handler);
}