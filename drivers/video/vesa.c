#include "include/vesa.h"
#include "include/types.h"
#include "include/memcore.h"
#include "include/mathlib.h"
#include "include/font.h"

// VBE info block is at 0x8000 in memory after boot
uint16_t* framebuffer;
static uint16_t vesa_width;
static uint16_t vesa_height;
static uint16_t vesa_pitch;

void vesa_init(uint16_t* fb, uint16_t w, uint16_t h, uint16_t p) {
    framebuffer = fb;
    vesa_width = w;
    vesa_height = h;
    vesa_pitch = p;
}

uint16_t vesa_get_width() {
    return vesa_width;
}

uint16_t vesa_get_height() {
    return vesa_height;
}

void vesa_put_pixel(int x, int y, uint16_t color) {
    if (framebuffer && x >= 0 && x < vesa_width && y >= 0 && y < vesa_height) {
        framebuffer[y * vesa_pitch / 2 + x] = color;
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

void vesa_draw_char(char c, int x, int y, uint16_t color) {
    const uint8_t* glyph = font[(int)c];
    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            if ((glyph[i] >> (7 - j)) & 1) {
                vesa_put_pixel(x + j, y + i, color);
            }
        }
    }
}