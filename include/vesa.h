#ifndef VESA_H
#define VESA_H

#include "include/types.h"

void vesa_init(uint32_t framebuffer_addr, uint16_t w, uint16_t h, uint16_t p);
void vesa_off();
void draw_char(int x, int y, char c, int color);

#endif