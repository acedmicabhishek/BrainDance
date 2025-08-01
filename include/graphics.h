#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "include/types.h"

extern uint32_t vga_trampoline_start[];
extern uint32_t vga_trampoline_end[];
extern uint32_t vga_trampoline_off_start[];
extern uint32_t vga_trampoline_off_end[];

void startx_vga();
void stopx_vga();
void put_pixel(int x, int y, uint8_t color);

#endif