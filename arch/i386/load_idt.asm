[bits 32]
global load_idt

load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret
