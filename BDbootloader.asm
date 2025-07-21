[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Load kernel (sector 2 = LBA 1)
    mov ah, 0x02
    mov al, 1         ; sectors to read
    mov ch, 0
    mov cl, 2         ; sector number
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x8000
    int 0x13
    jc disk_fail

    ; Enable A20
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; Load GDT
    cli
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

; GDT
gdt_start:
gdt_null:      dq 0
gdt_code:      dw 0xFFFF, 0, 0x9A00, 0xCF
gdt_data:      dw 0xFFFF, 0, 0x9200, 0xCF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    call 0x8000

halt:
    cli
    hlt
    jmp halt

disk_fail:
    mov si, disk_msg
print_error:
    lodsb
    or al, al
    jz halt
    mov ah, 0x0E
    int 0x10
    jmp print_error

disk_msg db "Disk read failed!", 0

CODE_SEG equ 0x08
DATA_SEG equ 0x10

times 510 - ($ - $$) db 0
dw 0xAA55
