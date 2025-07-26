#include "include/timer.h"
#include "include/irq.h"
#include "include/ports.h"
#include "include/memcore.h"

unsigned int timer_ticks = 0;

void timer_handler(struct regs *r) {
    timer_ticks++;

    if (timer_ticks % 100 == 0) {
        print("One second has passed\n", 0x02);
    }
}

void timer_install() {
    irq_install_handler(0, timer_handler);
}