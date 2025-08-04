#ifndef CABLE_H
#define CABLE_H

#include "include/pmm.h"
#include "include/keyboard.h"

#define EDITOR_ROWS 24
#define EDITOR_COLS 80
#define STATUS_BAR_COLOR 0x1F 


typedef struct {
    int cx, cy; 
    char buffer[EDITOR_ROWS][EDITOR_COLS];
    char filename[32];
    int dirty; 
} editor_state_t;


void cable_main(const char* filename);
void editor_init(const char* filename);
void editor_draw();
int editor_process_keypress();
void editor_insert_char(char c);
void editor_delete_char();
void editor_save_file();

#endif 