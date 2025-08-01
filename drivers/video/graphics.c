#include "include/graphics.h"

static graphics_mode_t current_mode = VGA_MODE;

void set_graphics_mode(graphics_mode_t mode) {
    current_mode = mode;
}

graphics_mode_t get_graphics_mode(void) {
    return current_mode;
}