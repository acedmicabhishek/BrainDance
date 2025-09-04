[org 0x7c00]
bits 16

jmp main

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
fs_type:            db "FAT12   "

main:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; Silent Countdown
    mov cx, 3
countdown_loop:
    push cx
    mov ah, 0x86
    mov cx, 0x0f
    mov dx, 0x4240
    int 0x15
    mov ah, 0x01
    int 0x16
    jz no_key
    mov ah, 0x00
    int 0x16
    cmp al, '2'
    je enter_bios_shell
no_key:
    pop cx
    loop countdown_loop
    jmp boot_stage_2

enter_bios_shell:
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    call clear_screen
    call show_info
shell_loop:
    mov si, prompt
    call print
    mov di, cmd_buf
read_loop:
    mov ah, 0x00
    int 0x16
    cmp al, 0x0d
    je process_cmd
    cmp al, 0x08
    je backspace
    mov ah, 0x0e
    int 0x10
    stosb
    jmp read_loop
backspace:
    cmp di, cmd_buf
    je read_loop
    dec di
    mov ah, 0x0e
    mov al, 0x08
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 0x08
    int 0x10
    jmp read_loop

process_cmd:
    mov byte [di], 0
    call newline
    mov si, cmd_buf
    mov di, start_cmd
    mov cx, 5
    repe cmpsb
    je boot_stage_2
    mov si, cmd_buf
    mov di, res_cmd
    mov cx, 4
    repe cmpsb
    je res_handler
    mov si, unknown_cmd
    call print
    jmp shell_loop

boot_stage_2:
    mov ah, 0x02
    mov al, 1
    mov cl, 2
    mov ch, 0
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x7e00
    int 0x13
    jc boot_fail
    jmp 0x7e00
boot_fail:
    mov si, boot_fail_msg
    call print
    jmp shell_loop
res_handler:
    mov si, res_msg
    call print
    jmp shell_loop

show_info:
    mov si, info_msg_vga
    call print
    mov ah, 0x0f
    int 0x10
    push ax
    mov al, ah
    call print_dec
    mov si, info_msg_mode
    call print
    pop ax
    call print_hex
    call newline
    ret

clear_screen:
    mov ah, 0x06
    xor al, al
    xor cx, cx
    mov dx, 0x184f
    mov bh, 0x07
    int 0x10
    ret
print:
    mov ah, 0x0e
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret
newline:
    mov ah, 0x0e
    mov al, 0x0d
    int 0x10
    mov al, 0x0a
    int 0x10
    ret
print_hex:
    mov cl, al
    mov ah, 0x0e
    shr al, 4
    add al, '0'
    cmp al, '9'
    jle ph_1
    add al, 7
ph_1:
    int 0x10
    mov al, cl
    and al, 0x0f
    add al, '0'
    cmp al, '9'
    jle ph_2
    add al, 7
ph_2:
    int 0x10
    ret
print_dec:
    mov ah, 0
    mov dl, 10
    div dl
    add ax, '00'
    mov dx, ax
    mov ah, 0x0e
    mov al, dl
    int 0x10
    mov al, dh
    int 0x10
    ret

prompt: db '> ', 0
cmd_buf: times 16 db 0
start_cmd: db 'start', 0
res_cmd:   db 'res', 0
unknown_cmd: db '?', 0x0d, 0x0a, 0
res_msg: db 'n/a', 0x0d, 0x0a, 0
info_msg_vga: db 'VGA: ', 0
info_msg_mode: db 'x25 Mode:0x', 0
boot_fail_msg: db '!', 0x0d, 0x0a, 0

times 510 - ($ - $$) db 0
dw 0xaa55