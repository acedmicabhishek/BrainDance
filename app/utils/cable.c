#include "include/cable.h"
#include "include/bdfs.h"
#include "include/memcore.h"
#include "include/colors.h"
#include "include/types.h"
#include "include/keyboard.h"
#include "include/exec.h"

static editor_state_t editor;

void syscall_write(const char* path, const uint8_t* buffer, uint32_t len) {
    uint8_t bytecode[1024];
    int ip = 0;
    bytecode[ip++] = OPCODE_SYSCALL_WRITE;
    strcpy((char*)&bytecode[ip], path);
    ip += strlen(path) + 1;
    *(uint32_t*)&bytecode[ip] = len;
    ip += sizeof(uint32_t);
    memcpy(&bytecode[ip], buffer, len);
    ip += len;
    bytecode[ip++] = OPCODE_EXIT;
    interpret_bdx(bytecode);
}

void editor_init(const char* filename) {
    editor.cx = 0;
    editor.cy = 0;
    editor.dirty = 0;
    strncpy(editor.filename, filename, sizeof(editor.filename) - 1);
    memset(editor.buffer, 0, sizeof(editor.buffer));

    // Load file content if it exists
    uint32_t bytes_read;
    uint8_t file_content[EDITOR_ROWS * EDITOR_COLS];
    if (bdfs_read_file(filename, file_content, &bytes_read) == 0) {
        int row = 0;
        int col = 0;
        for (uint32_t i = 0; i < bytes_read && row < EDITOR_ROWS; i++) {
            if (file_content[i] == '\n') {
                row++;
                col = 0;
            } else if (col < EDITOR_COLS - 1) {
                editor.buffer[row][col++] = file_content[i];
            }
        }
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
    uint8_t save_buffer[EDITOR_ROWS * EDITOR_COLS];
    for (int y = 0; y < EDITOR_ROWS; y++) {
        int row_len = strlen(editor.buffer[y]);
        if (row_len > 0) {
            memcpy(save_buffer + len, editor.buffer[y], row_len);
            len += row_len;
            if (y < EDITOR_ROWS - 1) {
                save_buffer[len] = '\n';
                len++;
            }
        }
    }

    syscall_write(editor.filename, save_buffer, len);
    editor.dirty = 0;
}

int editor_process_keypress() {
    unsigned char scancode = keyboard_get_scancode();

    if (scancode) {
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