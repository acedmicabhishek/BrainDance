[BITS 16]
[ORG 0x7C00]

start:
    ; --- Basic Setup ---
    cli             ; Disable interrupts
    xor ax, ax      ; Zero out AX register
    mov ds, ax      ; Set data segment to 0
    mov es, ax      ; Set extra segment to 0
    mov ss, ax      ; Set stack segment to 0
    mov sp, 0x7C00  ; Set stack pointer below bootloader

    ; --- Load Kernel from Disk ---
    ; Uses BIOS interrupt 0x13 to read from the disk
    mov ah, 0x42      ; Function 42h: Extended Read
    mov dl, 0x80      ; Drive number (must be set)
    mov si, dap       ; DS:SI points to the DAP
    int 0x13          ; Call BIOS disk services
    jc halt           ; If carry flag is set, halt

    ; --- Read E820 Memory Map ---
    xor ebx, ebx      ; Start with EBX = 0
    mov edi, 0x1000   ; Buffer to store map entries
    mov dword [0x900], 0 ; Counter for map entries
read_map:
    mov eax, 0xE820   ; E820 function
    mov ecx, 20       ; Size of the buffer (20 bytes)
    mov edx, 0x534D4150 ; 'SMAP' signature
    int 0x15          ; Call BIOS
    jc halt           ; If error, halt

    add edi, 20       ; Move to next buffer position
    inc dword [0x900] ; Increment entry count

    cmp ebx, 0        ; If EBX is 0, we are done
    jne read_map      ; Otherwise, continue reading

    ; --- Enable A20 Line ---
    ; This allows access to memory above 1MB
    mov ax, 0x2401    ; BIOS A20 gate enable function
    int 0x15          ; Call BIOS services
    jc halt           ; If carry flag is set, A20 enable failed

    ; --- Load Global Descriptor Table (GDT) ---
    cli             ; Disable interrupts before loading GDT
    lgdt [gdt_descriptor] ; Load the GDT descriptor

    ; --- Enter 32-bit Protected Mode ---
    mov eax, cr0      ; Move control register 0 to EAX
    or eax, 1         ; Set the protection enable bit
    mov cr0, eax      ; Write back to CR0
    jmp CODE_SEG:init_pm ; Far jump to the 32-bit code segment

; --- Global Descriptor Table (GDT) ---
gdt_start:
gdt_null:      dq 0  ; Null descriptor
gdt_code:      dw 0xFFFF, 0, 0x9A00, 0xCF ; 4GB code segment
gdt_data:      dw 0xFFFF, 0, 0x9200, 0xCF ; 4GB data segment
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; GDT size
    dd gdt_start               ; GDT start address

[bits 32]
init_pm:
    ; --- Setup 32-bit Segments ---
    mov ax, DATA_SEG ; Load data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000 ; Set stack pointer for the kernel

    ; --- Copy Kernel to High Memory ---
    ; The kernel is loaded at 0x8000, but the linker expects it at 0x100000.
    mov esi, 0x8000      ; Source address
    mov edi, 0x100000    ; Destination address
    mov ecx, 512 * 128   ; Number of bytes to copy (64KB)
    cld                  ; Clear direction flag (for forward copying)
    rep movsb            ; Repeat move byte string

    ; --- Jump to Kernel ---
    call 0x100000        ; Call the kernel's entry point

halt:
    ; --- Halt the CPU ---
    cli
    hlt
    jmp halt


dap:
    db 0x10  ; Size of packet (16 bytes)
    db 0     ; Reserved
    dw 128   ; Number of sectors to read
    dw 0x8000; Destination buffer offset
    dw 0     ; Destination buffer segment
    dq 1     ; Start LBA (Logical Block Address)

; --- GDT Segment Selectors ---
CODE_SEG equ 0x08
DATA_SEG equ 0x10

; --- Bootloader Signature ---
times 510 - ($ - $$) db 0 ; Pad the bootloader to 510 bytes
dw 0xAA55                 ; Boot signature
