[BITS 16]
[ORG 0x7C00]
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov ah, 0x02
    mov al, 128
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, 0x80
    mov bx, 0x8000
    int 0x13
    jc halt
    xor ebx, ebx
    mov edi, 0x1000
    mov dword [0x900], 0
read_map:
    mov eax, 0xE820
    mov ecx, 20
    mov edx, 0x534D4150
    int 0x15
    jc halt
    add edi, 20
    inc dword [0x900]
    cmp ebx, 0
    jne read_map
    mov ax, 0x2401
    int 0x15
    jc halt
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm
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
    mov esi, 0x8000
    mov edi, 0x100000
    mov ecx, 512 * 128
    cld
    rep movsb
    call 0x100000
halt:
    cli
    hlt
    jmp halt
CODE_SEG equ 0x08
DATA_SEG equ 0x10
times 510 - ($ - $$) db 0
dw 0xAA55
