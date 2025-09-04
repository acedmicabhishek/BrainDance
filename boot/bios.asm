[org 0x7c00]
bits 16

jmp main

; BIOS Parameter Block (BPB)
times 3 - ($ - $$) db 0
oem_identifier:     db "BRAINBLD"
bytes_per_sector:   dw 512
sectors_per_cluster:db 1
reserved_sectors:   dw 1
num_fats:           db 2
num_root_entries:   dw 224
total_sectors:      dw 2880
media_descriptor:   db 0xf0
sectors_per_fat:    dw 9
sectors_per_track:  dw 18
num_heads:          dw 2
hidden_sectors:     dd 0
large_sector_count: dd 0
drive_number:       db 0
reserved:           db 0
signature:          db 0x29
volume_id:          dd 0
volume_label:       db "BRAINDANCE"
fs_type:            db "FAT12"

main:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Set video mode to text 80x25
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    call display_welcome

shell_loop:
    mov si, prompt
    call print_string

    ; Read command
    mov di, command_buffer
read_char:
    mov ah, 0x00
    int 0x16

    cmp al, 0x0d ; Enter
    je process_command

    cmp al, 0x08 ; Backspace
    je handle_backspace

    ; Echo character
    mov ah, 0x0e
    int 0x10

    stosb
    jmp read_char

handle_backspace:
    cmp di, command_buffer
    je read_char
    dec di
    mov byte [di], 0
    mov ah, 0x0e
    mov al, 0x08
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 0x08
    int 0x10
    jmp read_char

process_command:
    mov byte [di], 0 ; Null-terminate the command
    call print_newline

    ; Compare with "start"
    mov si, command_buffer
    mov di, start_cmd
    mov cx, 5 ; 'start' length + null
    repe cmpsb
    je .start

    ; Compare with "res"
    mov si, command_buffer
    mov di, res_cmd
    mov cx, 4 ; 'res' length + null
    repe cmpsb
    je .res

    ; Compare with "help"
    mov si, command_buffer
    mov di, help_cmd
    mov cx, 5 ; 'help' length + null
    repe cmpsb
    je .help

    ; Compare with "info"
    mov si, command_buffer
    mov di, info_cmd
    mov cx, 5 ; 'info' length + null
    repe cmpsb
    je .info

    ; Compare with "cls"
    mov si, command_buffer
    mov di, cls_cmd
    mov cx, 4 ; 'cls' length + null
    repe cmpsb
    je .cls

    mov si, unknown_cmd_msg
    call print_string
    jmp shell_loop

.start:
    ; Load the second stage bootloader from disk
    mov ah, 0x02      ; Function 02h: Read Sectors
    mov al, 1         ; Number of sectors to read
    mov ch, 0         ; Cylinder number
    mov cl, 2         ; Starting sector number
    mov dh, 0         ; Head number
    mov dl, 0x80      ; Drive number
    mov bx, 0x7e00    ; Destination buffer
    int 0x13          ; Call BIOS disk services
    jc .start_fail    ; If carry flag is set, it failed

    jmp 0x7e00

.start_fail:
    mov si, boot_fail_msg
    call print_string
    jmp shell_loop

.res:
    mov si, res_msg
    call print_string
    jmp shell_loop

.help:
    mov si, help_msg
    call print_string
    jmp shell_loop

.info:
    mov si, info_msg
    call print_string
    jmp shell_loop

.cls:
    call display_welcome
    jmp shell_loop

display_welcome:
    call clear_screen
    mov si, welcome_msg
    call print_string
    ret

clear_screen:
    mov ah, 0x06
    mov al, 0
    mov bh, 0x07
    mov cx, 0
    mov dx, 0x184f
    int 0x10
    ret

print_string:
    mov ah, 0x0e
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

print_newline:
    mov ah, 0x0e
    mov al, 0x0d
    int 0x10
    mov al, 0x0a
    int 0x10
    ret

welcome_msg: db 'BD-BIOS', 0x0d, 0x0a, 0
prompt: db '> ', 0
command_buffer: times 32 db 0
start_cmd: db 'start', 0
res_cmd: db 'res', 0
help_cmd: db 'help', 0
info_cmd: db 'info', 0
cls_cmd: db 'cls', 0
unknown_cmd_msg: db 'Unknown cmd', 0x0d, 0x0a, 0
res_msg: db 'Not implemented', 0x0d, 0x0a, 0
help_msg: db 'start, res, info, cls', 0x0d, 0x0a, 0
info_msg: db 'BD-BIOS v1.0, 80x25 Text', 0x0d, 0x0a, 0
boot_fail_msg: db 'Boot failed!', 0x0d, 0x0a, 0

times 510 - ($ - $$) db 0
dw 0xaa55