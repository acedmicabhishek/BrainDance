#pragma once

#include "regs.h"
#include "pmm.h"

typedef void (*isr_t)(regs_t*);
void register_interrupt_handler(uint8_t n, isr_t handler);
void isr_install();