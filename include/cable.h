#ifndef CABLE_H
#define CABLE_H

#include "include/pmm.h"
#include "include/keyboard.h"

#define EDITOR_ROWS 24
#define EDITOR_COLS 80
#define STATUS_BAR_COLOR 0x1F // White text on Blue background

// Represents the state of the editor
typedef struct {
    int cx, cy; // Cursor x and y position
    char buffer[EDITOR_ROWS][EDITOR_COLS];
    char filename[32];
    int dirty; // Flag to track if the file has been modified
} editor_state_t;

// Function prototypes
void cable_main(const char* filename);
void editor_init(const char* filename);
void editor_draw();
int editor_process_keypress();
void editor_insert_char(char c);
void editor_delete_char();
void editor_save_file();

#endif // CABLE_H