#ifndef SHELL_H
#define SHELL_H

void start_shell();
void process_command(const char* command);
void print_prompt();

// Command handlers
void help_command();
void clear_command();
void meminfo_command();
void time_command();
void halt_command();
void sysinfo_command();
void apps_command();
void ls_command();
void touch_command(const char* filename);
void write_command(const char* filename, const char* data);
void cat_command(const char* filename);
void echo_command(const char* text);
void rm_command(const char* filename);
void mv_command(const char* old_filename, const char* new_filename);
void mkdir_command(const char* dirname);
void cd_command(const char* dirname);
void format_command();
void ataread_command(const char* lba_str);
void atawrite_command(const char* lba_str, const char* data);

#endif