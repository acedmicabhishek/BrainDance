#ifndef COLORS_H
#define COLORS_H

// -- BrainDance OS Color Theme --
// we are stuck with a limited 16-color palette, we might switch off VGA

// Custom Palette Reference:
// #f8e602 (Yellow), #4bff21 (Lime Green), #00f0ff (Neon Cyan), 
// #f4d5fd (Pale Pink/Magenta), #772289 (Deep Purple)

// General UI
#define COLOR_PROMPT       0x0B // Neon Cyan (#00f0ff)
#define COLOR_INPUT        0x0B // Same Neon Cyan for input
#define COLOR_SYSTEM       0x05 // Deep Purple (#772289)
#define COLOR_WARNING      0x0D // Pale Pink (#f4d5fd)
#define COLOR_SUCCESS      0x0A // Lime Green (#4bff21)
#define COLOR_ERROR        0x04 // Keep standard Red for clarity
#define COLOR_ATA_LOG      0x0E // Bright Yellow (#f8e602)

// Filesystem Colors
#define COLOR_DIR          0x0E // Bright Yellow
#define COLOR_FILE         0x07 // White / default gray
#define COLOR_GHOST        0x09 // Pale Blue / Ghost bin

// Directory-specific colors
#define COLOR_DIR_SOUL     0x0B // Cyan for soul (OS shell)
#define COLOR_DIR_CORTEX   0x05 // Deep Purple for cortex
#define COLOR_DIR_VAULT    0x0E // Yellow for personal data vault
#define COLOR_DIR_CHROME   0x0A // Green for hardware interaction
#define COLOR_DIR_DRIFT    0x0D // Magenta log stream
#define COLOR_DIR_GHOST    0x09 // Ghost bin / deleted

#endif
