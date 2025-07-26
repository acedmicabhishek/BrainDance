ENTRY(kernel_main)

SECTIONS {
    /* Set kernel load address to 1MB */
    . = 0x100000;

    /* Code section */
    .text ALIGN(4) : {
        *(.text)
    }

    /* Read-only data */
    .rodata ALIGN(4) : {
        *(.rodata)
    }

    /* Initialized data */
    .data ALIGN(4) : {
        *(.data)
    }

    /* Uninitialized data (zero-initialized) */
    .bss ALIGN(4) : {
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }

    /* Stack reservation (32KB, aligned to 16 bytes) */
    .stack ALIGN(16) : {
        _stack_start = .;
        . = . + 0x8000;
        _stack_end = .;
    }
}
