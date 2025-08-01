#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef enum {
    VGA_MODE,
    VESA_MODE
} graphics_mode_t;

void set_graphics_mode(graphics_mode_t mode);
graphics_mode_t get_graphics_mode(void);

#endif