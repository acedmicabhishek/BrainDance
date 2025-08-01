#ifndef VESA_H
#define VESA_H

#include "include/types.h"

// VBE Mode Information Block
typedef struct {
    uint16_t attributes;
    uint8_t window_a_attributes;
    uint8_t window_b_attributes;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t window_a_segment;
    uint16_t window_b_segment;
    uint32_t window_function_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t char_width;
    uint8_t char_height;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;
    uint8_t red_mask_size;
    uint8_t red_field_position;
    uint8_t green_mask_size;
    uint8_t green_field_position;
    uint8_t blue_mask_size;
    uint8_t blue_field_position;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_position;
    uint8_t direct_color_attributes;
    uint32_t framebuffer;
    uint32_t off_screen_mem_offset;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed)) vbe_mode_info_t;

void vesa_init(uint16_t* fb, uint16_t w, uint16_t h, uint16_t p);
void vesa_put_pixel(int x, int y, uint16_t color);
void vesa_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color);
void vesa_draw_char(char c, int x, int y, uint16_t color);
uint16_t vesa_get_width();
uint16_t vesa_get_height();

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

#endif // VESA_H