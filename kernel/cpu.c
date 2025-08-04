#include "include/cpu.h"
#include "include/timer.h"

static uint32_t idle_ticks = 0;
static uint32_t total_ticks = 0;

void cpu_init() {
    // Nothing to do here for now bitch
}

void cpu_idle() {
    asm volatile("sti");
    asm volatile("hlt");
    idle_ticks++;
}

int cpu_get_usage() {
    if (total_ticks == 0) {
        return 0;
    }
    return 100 - (idle_ticks * 100) / total_ticks;
}

void cpu_tick() {
    total_ticks++;
}