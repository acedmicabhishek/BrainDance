# BrainDance OS Documentation

This document provides a detailed explanation of the BrainDance operating system, covering its architecture, components, and functionality.

## 1. Boot Process

The boot process begins with the BIOS loading and executing our bootloader, `boot/BDbootloader.asm`. This bootloader operates in **16-bit real mode**, which is the default state of x86 CPUs at startup. Our primary goal is to transition the system to **32-bit protected mode**, where our C kernel can run.

### Key Responsibilities:
- **Initial Setup:** Disables interrupts (`cli`) and sets up segment registers (`ds`, `es`, `ss`) to 0.
- **Kernel Loading:** Uses BIOS interrupt `0x13` to read 64 sectors (32KB) of the kernel from the disk into memory at address `0x8000`.
- **Enable A20 Line:** Activates the A20 gate to allow access to memory above 1MB.
- **Enter Protected Mode:**
    1.  Loads the Global Descriptor Table (GDT) using the `lgdt` instruction. The GDT defines memory segments for code and data.
    2.  Sets the Protection Enable (PE) bit in the `CR0` register.
    3.  Performs a far jump to our 32-bit code segment to flush the CPU pipeline.
- **Kernel Relocation:** Copies the kernel from its temporary location at `0x8000` to its final destination at `0x100000` (1MB), as defined in the linker script.
- **Jump to Kernel:** Calls the `kernel_main` function at `0x100000`, transferring control to our 32-bit C kernel.

## 2. Kernel

The kernel entry point is the `kernel_main` function in `kernel/BDkernel.c`.

### Initialization Sequence:
1.  **Screen Clear:** Clears the VGA text buffer to provide a clean display.
2.  **Memory Diagnostics:** Calculates and prints the size of the BSS (uninitialized data) and stack sections.
3.  **Interrupts:**
    -   `idt_install()`: Initializes the Interrupt Descriptor Table.
    -   `isr_install()`: Sets up handlers for the first 32 CPU exceptions.
    -   `irq_install()`: Installs handlers for the 16 hardware interrupts (IRQs).
4.  **Timer:**
    -   `timer_install()`: Initializes the Programmable Interval Timer (PIT) to fire at 100 Hz.
5.  **Enable Interrupts:** Executes the `sti` instruction to allow the CPU to respond to interrupts.
6.  **Idle Loop:** Enters an infinite loop (`for(;;);`) to wait for interrupts.

## 3. Memory Core Library (`libc/`)

This directory contains our standard library functions.

- **`memcore.c` / `include/memcore.h`:**
    -   `memcpy`, `memset`, `strlen`: Standard memory and string manipulation functions.
    -   `clear_screen`, `print`, `print_int`, `print_hex`: Functions for writing to the VGA text-mode buffer.
    -   `kprintf`: A simple `printf` implementation supporting `%d`, `%x`, `%s`, and `%c`.
    -   `panic`: Halts the system and displays a critical error message.
    -   `log`: A utility for printing formatted log messages.

## 4. Interrupt Handling (`arch/i386/`)

This directory contains the low-level, architecture-specific code for handling interrupts and exceptions on the i386 platform.

### Interrupt Descriptor Table (IDT)
- **`idt.c` / `idt.h`:**
    -   Defines the `idt_entry` and `idt_ptr` structures.
    -   `idt_set_gate()`: Populates an entry in the IDT with a handler's address and attributes.
    -   `idt_install()`: Initializes the IDT and loads it using the `lidt` assembly instruction.
- **`load_idt.asm`:** Contains the `load_idt` function that executes the `lidt` instruction.

### Interrupt Service Routines (ISRs)
- **`isr.c` / `isr.h`:**
    -   Handles the first 32 CPU exceptions (e.g., Divide by Zero, Page Fault).
    -   `isr_install()`: Populates the IDT with handlers for each of the 32 exceptions.
    -   `isr_handler()`: The common C handler that receives the interrupt number and prints an exception message.
- **`isr.asm`:**
    -   Uses macros (`ISR_NOERR`, `ISR_ERR`) to generate assembly stubs for each of the 32 exceptions.
    -   `isr_common_stub`: The common assembly stub that saves the processor state, calls the C handler, and restores the state before returning.

### Hardware Interrupts (IRQs)
- **`irq.c` / `irq.h`:**
    -   Manages the 16 hardware interrupts from devices like the timer and keyboard.
    -   `irq_install_handler()`: Associates a C function with a specific IRQ number.
    -   `irq_handler()`: The common C dispatcher that calls the appropriate handler for a given IRQ.
- **`irq.asm`:**
    -   Contains assembly stubs for each of the 16 IRQs, which call the common `irq_handler`.

### Programmable Interrupt Controller (PIC)
- **`pic.c` / `include/pic.h`:**
    -   `pic_remap()`: Re-programs the master and slave PICs to map IRQs 0-15 to IDT entries 32-47. This is crucial to avoid conflicts with the CPU exceptions, which occupy the first 32 entries.
    -   `pic_send_eoi()`: Sends the "End-Of-Interrupt" signal to the PIC after an IRQ has been handled.

## 5. Timer
- **`timer.c` / `include/timer.h`:**
    -   `timer_install()`: Initializes the Programmable Interval Timer (PIT) to generate an interrupt at a frequency of 100 Hz.
    -   `timer_handler()`: The interrupt handler for the timer (IRQ0). It increments a global `timer_ticks` counter and prints a message every 100 ticks (1 second) to demonstrate that it's working.

## 6. Build System (`Makefile`)
The `Makefile` automates the entire build process:
- Compiles all C and assembly files into object files.
- Links the object files together to create the final kernel executable (`BDkernel.elf`).
- Converts the ELF executable to a flat binary file (`BDkernel.bin`).
- Concatenates the bootloader and the kernel binary to create a bootable disk image (`bdos.img`).