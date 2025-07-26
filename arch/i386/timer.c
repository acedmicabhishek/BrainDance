#include "include/timer.h"
#include "include/irq.h"
#include "include/ports.h"
#include "include/memcore.h"

unsigned int timer_ticks = 0;

void timer_handler(struct regs *r) {
    timer_ticks++;

    if (timer_ticks % 100 == 0) {
        // print("One second has passed\n", 0x02);
    }
}

void timer_install() {
    // IRQ0 will be fired at 100Hz
    unsigned int divisor = 1193182 / 100;

    // Send the command byte
    outb(0x43, 0x36);

    // Send the divisor
    unsigned char l = (unsigned char)(divisor & 0xFF);
    unsigned char h = (unsigned char)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);

    // Install the handler
    irq_install_handler(0, timer_handler);
}