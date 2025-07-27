#pragma once

#include "regs.h"
#include "pmm.h"
extern uint32_t timer_ticks; // Declare timer_ticks as external

void timer_install();
void timer_handler(regs_t *r);