#include "include/vesa.h"
#include "include/types.h"
#include "include/memcore.h"
#include "include/mathlib.h"

// VBE info block is at 0x8000 in memory after boot
vbe_mode_info_t* vbe_mode_info = (vbe_mode_info_t*)0x8000;
uint16_t* framebuffer;

void vesa_init() {
    framebuffer = (uint16_t*)vbe_mode_info->framebuffer;
}

uint16_t vesa_get_width() {
    return vbe_mode_info->width;
}

uint16_t vesa_get_height() {
    return vbe_mode_info->height;
}

void vesa_put_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < vbe_mode_info->width && y >= 0 && y < vbe_mode_info->height) {
        framebuffer[y * vbe_mode_info->pitch / 2 + x] = color;
    }
}

// Helper function to draw a line
static void draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        vesa_put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Function to draw a triangle
void vesa_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) {
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x3, y3, color);
    draw_line(x3, y3, x1, y1, color);
}