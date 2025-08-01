#include "include/vesa.h"
#include "include/memcore.h"
#include "include/font.h"

#define FONT_WIDTH 8
#define FONT_HEIGHT 16


static uint32_t* framebuffer;
static uint16_t width;
static uint16_t height;
static uint16_t pitch;

void vesa_init(uint32_t framebuffer_addr, uint16_t w, uint16_t h, uint16_t p) {
    framebuffer = (uint32_t*)framebuffer_addr;
    width = w;
    height = h;
    pitch = p;
}

void vesa_off() {
    // This is a placeholder. The actual mode setting will be done via a real-mode trampoline.
}

void draw_char(int x, int y, char c, int color) {
    if (x >= width || y >= height) {
        return;
    }

    const uint8_t* glyph = font_data[(uint8_t)c];
    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            if ((glyph[i] >> (FONT_WIDTH - 1 - j)) & 1) {
                framebuffer[(y + i) * (pitch / 4) + (x + j)] = color;
            }
        }
    }
}

void vesa_clear_screen(uint32_t color) {
    for (uint32_t i = 0; i < width * height; i++) {
        framebuffer[i] = color;
    }
}

void put_pixel(int x, int y, uint32_t color) {
    if (x >= width || y >= height) {
        return;
    }
    framebuffer[y * (pitch / 4) + x] = color;
}

uint16_t get_vesa_width() {
    return width;
}

uint16_t get_vesa_height() {
    return height;
}

uint16_t get_vesa_pitch() {
    return pitch;
}

uint32_t* get_vesa_framebuffer() {
    return framebuffer;
}