#include "include/cable.h"
#include "include/bdfs.h"
#include "include/memcore.h"
#include "include/colors.h"
#include "include/types.h"
#include "include/keyboard.h"

static editor_state_t editor;

void editor_init(const char* filename) {
    editor.cx = 0;
    editor.cy = 0;
    editor.dirty = 0;
    strncpy(editor.filename, filename, sizeof(editor.filename) - 1);
    memset(editor.buffer, 0, sizeof(editor.buffer));

    // Load file content if it exists
    uint32_t bytes_read;
    if (bdfs_read_file(filename, (uint8_t*)editor.buffer, &bytes_read) == 0) {
        // For simplicity, we assume the file fits into our buffer.
        // A more robust implementation would handle larger files.
    }
}

void editor_draw_status_bar() {
    char status[EDITOR_COLS];
    char dirty_indicator = editor.dirty ? '*' : ' ';
    snprintf(status, sizeof(status), "-- CABLE -- [%s]%c  Ctrl+S=Save | Ctrl+Q=Quit", editor.filename, dirty_indicator);
    
    for (int i = 0; i < EDITOR_COLS; i++) {
        if (i < strlen(status)) {
            print_char_at(status[i], STATUS_BAR_COLOR, i, EDITOR_ROWS - 1);
        } else {
            print_char_at(' ', STATUS_BAR_COLOR, i, EDITOR_ROWS - 1);
        }
    }
}

void editor_draw() {
    clear_screen(COLOR_SYSTEM);
    for (int y = 0; y < EDITOR_ROWS - 1; y++) {
        for (int x = 0; x < EDITOR_COLS; x++) {
            print_char_at(editor.buffer[y][x], COLOR_INPUT, x, y);
        }
    }
    editor_draw_status_bar();
    set_cursor(editor.cx, editor.cy);
}

void editor_insert_char(char c) {
    if (editor.cx < EDITOR_COLS - 1) {
        editor.buffer[editor.cy][editor.cx] = c;
        editor.cx++;
        editor.dirty = 1;
    }
}

void editor_delete_char() {
    if (editor.cx > 0) {
        editor.cx--;
        editor.buffer[editor.cy][editor.cx] = '\0';
        editor.dirty = 1;
    }
}

void editor_save_file() {
    uint32_t len = 0;
    for (int y = 0; y < EDITOR_ROWS; y++) {
        len += strlen(editor.buffer[y]);
    }

    if (bdfs_write_file(editor.filename, (uint8_t*)editor.buffer, len) > 0) {
        editor.dirty = 0;
    }
}

int editor_process_keypress() {
    unsigned char scancode = keyboard_get_scancode();

    // Ctrl+S to save
    if (scancode == 0x1F && ctrl_pressed) {
        editor_save_file();
        return 1; // Continue running
    }
    
    // Ctrl+Q to quit
    if (scancode == 0x10 && ctrl_pressed) {
        // For now, we just exit. A real implementation would check for unsaved changes.
        return 0; // Signal to quit
    }

    char c = kbd_us[scancode];
    if (c) {
        switch (c) {
            case '\n':
                editor.cy++;
                editor.cx = 0;
                break;
            case '\b':
                editor_delete_char();
                break;
            default:
                if (c >= 32 && c <= 126) {
                    editor_insert_char(c);
                }
                break;
        }
    }
    return 1; // Continue running
}

void cable_main(const char* filename) {
    editor_init(filename);
    
    int running = 1;
    while (running) {
        editor_draw();
        running = editor_process_keypress();
    }
}