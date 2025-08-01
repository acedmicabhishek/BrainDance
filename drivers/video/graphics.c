#include "include/graphics.h"
#include "include/memcore.h"
#include "include/colors.h"

void startx_vga() {
    println("Switching to VGA graphics mode...", COLOR_SYSTEM);
    
    // Copy the trampoline to a known, safe address in low memory
    memcpy((void*)0x7000, &vga_trampoline_start, (uint32_t)&vga_trampoline_end - (uint32_t)&vga_trampoline_start);
    
    // Call the trampoline
    ((void (*)(void))0x7000)();
    
    // Draw a test pixel
    put_pixel(100, 100, 0x1C); // Cyan
}

void stopx_vga() {
    println("Switching to text mode...", COLOR_SYSTEM);
    
    // Copy the off trampoline
    memcpy((void*)0x7000, &vga_trampoline_off_start, (uint32_t)&vga_trampoline_off_end - (uint32_t)&vga_trampoline_off_start);
    
    // Call the trampoline
    ((void (*)(void))0x7000)();
}

void put_pixel(int x, int y, uint8_t color) {
    uint8_t* vga = (uint8_t*)0xA0000;
    vga[y * 320 + x] = color;
}