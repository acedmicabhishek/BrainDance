# BrainDance OS Documentation

This document provides a detailed explanation of the BrainDance operating system, covering its architecture, components, and functionality.

## 1. Boot Process

The boot process is now a two-stage process, starting with an interactive BIOS shell.

### Stage 1: BIOS Shell (`boot/bios.asm`)
The boot process begins with the BIOS loading and executing our first-stage bootloader, `boot/bios.asm`. This stage operates in **16-bit real mode** and presents the user with a simple, interactive command shell.

#### Key Responsibilities:
- **Initial Setup:** Sets up segment registers and initializes the VGA text mode.
- **Interactive Shell:** Provides a command prompt where the user can issue commands.
  - `help`: Lists available commands.
  - `info`: Displays basic BIOS and system information.
  - `cls`: Clears the screen.
  - `start`: Proceeds to the second stage of the boot process.
- **Loading Stage 2:** When the `start` command is executed, this bootloader reads the second sector of the disk into memory at `0x7e00` and jumps to it.

### Stage 2: Kernel Loader (`boot/BDbootloader.asm`)
The second-stage bootloader is responsible for preparing the system for the C kernel and loading it into memory.

#### Key Responsibilities:
- **E820 Memory Map:** Reads the system's memory map using the BIOS `0xE820` interrupt and stores it at address `0x1000`.
- **Kernel Loading:** Uses BIOS interrupt `0x13` to read 128 sectors (64KB) of the kernel from the disk (starting at sector 3) into memory at address `0x8000`.
- **Enable A20 Line:** Activates the A20 gate to allow access to memory above 1MB.
- **Enter Protected Mode:**
    1.  Loads the Global Descriptor Table (GDT).
    2.  Sets the Protection Enable (PE) bit in the `CR0` register.
    3.  Performs a far jump to the 32-bit code segment.
- **Kernel Relocation:** Copies the kernel from `0x8000` to its final destination at `0x100000` (1MB).
- **Jump to Kernel:** Calls the `kernel_main` function at `0x100000`, transferring control to our 32-bit C kernel.

## 2. Kernel

The kernel entry point is the `kernel_main` function in `kernel/BDkernel.c`.

### Initialization Sequence:
1.  **Physical Memory Manager (PMM):** Initializes the PMM using the memory map provided by the bootloader.
2.  **Screen Clear:** Clears the VGA text buffer.
3.  **Interrupts:**
    -   `idt_install()`: Initializes the Interrupt Descriptor Table.
    -   `isr_install()`: Sets up handlers for the first 32 CPU exceptions.
    -   `irq_install()`: Installs handlers for the 16 hardware interrupts (IRQs).
4.  **Paging:** `paging_install()` enables paging and sets up the initial page directory.
5.  **Timer:** `timer_install()` initializes the Programmable Interval Timer (PIT) to fire at 100 Hz.
6.  **Keyboard:** `keyboard_install()` sets up the keyboard driver.
7.  **ATA Driver:** `ata_init()` initializes the ATA driver for disk access.
8.  **Filesystem:** `bdfs_init()` initializes the BrainDance File System (BDFS).
9.  **PCI Bus:** `pci_scan_all()` scans the PCI bus for connected devices.
10. **CPU Monitoring:** `cpu_init()` initializes CPU usage monitoring.
11. **Enable Interrupts:** Executes the `sti` instruction.
12. **Start Shell:** Calls `start_shell()` to launch the user interface.

## 3. Memory Core Library (`libc/`)

- **`memcore.c` / `include/memcore.h`:**
    -   `memcpy`, `memset`, `strlen`, `strcmp`, `strcpy`, `strtok`: Standard memory and string manipulation functions.
    -   `clear_screen`, `print`, `print_char`, `print_int`, `print_hex`, `print_backspace`, `scroll_up`, `scroll_down`: Functions for writing to the VGA text-mode buffer.
    -   `kprintf`: A simple `printf` implementation supporting `%d`, `%x`, `%s`, and `%c`.
    -   `panic`: Halts the system and displays a critical error message.
    -   `log`: A utility for printing formatted log messages.

## 4. Interrupt Handling (`arch/i386/`)

This directory contains the low-level, architecture-specific code for handling interrupts and exceptions.

### Interrupt Descriptor Table (IDT)
- **`idt.c` / `idt.h`:** Defines and installs the IDT. `idt_set_gate()` is used to populate the IDT with interrupt handlers.

### Interrupt Service Routines (ISRs)
- **`isr.c` / `isr.h` / `isr.asm`:** Handles the first 32 CPU exceptions. Assembly stubs save processor state and call a common C handler (`isr_handler`).

### Hardware Interrupts (IRQs)
- **`irq.c` / `irq.h` / `irq.asm`:** Manages the 16 hardware interrupts. It remaps the PIC, installs handlers for each IRQ, and dispatches interrupts to the correct C function.

### Programmable Interrupt Controller (PIC)
- **`pic.c` / `include/pic.h`:** `pic_remap()` re-programs the master and slave PICs to avoid conflicts with CPU exceptions.

## 5. Timer
- **`timer.c` / `include/timer.h`:**
    -   `timer_install()`: Initializes the PIT to generate an interrupt (IRQ0) at 100 Hz.
    -   `timer_handler()`: The interrupt handler for the timer. It increments a global `timer_ticks` counter and calls `cpu_tick()` for usage monitoring.

## 6. Memory Management

### 6.1. Physical Memory Manager (PMM) (`memory/pmm.c`)
The PMM is responsible for managing the physical memory of the system. It uses the E820 memory map to keep track of which physical memory pages are free and which are in use.
- `pmm_init()`: Initializes the PMM with the memory map from the bootloader.
- `pmm_alloc_block()`: Allocates a single 4KB block of physical memory.
- `pmm_free_block()`: Frees a 4KB block of physical memory.

### 6.2. Paging (`memory/paging.c`)
Paging enables virtual memory, providing each process with its own isolated address space.
- `paging_install()`: Initializes paging, identity-maps the first 4MB of memory, and installs the page fault handler. It uses a recursive mapping technique to access page tables.
- `map_page()` / `unmap_page()`: Functions to manage virtual to physical address mappings.

### 6.3. Heap (`memory/heap.c`)
The kernel heap provides dynamic memory allocation using a simple bump allocator.
- `kmalloc()`: Allocates a block of memory from the heap.
- `kfree()`: A stub function, as the bump allocator does not support individual deallocations.

## 7. Drivers

### 7.1. Keyboard Driver (`drivers/keyboard_driver.c`)
Handles input from a PS/2 keyboard, converting scancodes to ASCII characters. It now supports arrow keys and the Ctrl key.

### 7.2. ATA Driver (`drivers/ata/ata.c`)
Provides an interface for reading from and writing to IDE hard drives using Programmed I/O (PIO).

### 7.3. PCI Driver (`network/pci.c`)
- `pci_scan_all()`: Scans the PCI bus for all connected devices and prints their information.

### 7.4. E1000 Network Driver (`network/e1000.c`)
A driver for the Intel E1000 network card (work in progress).

## 8. Filesystem (BDFS)

BrainDance OS includes a simple, in-memory filesystem called BDFS (BrainDance File System).

### 8.1. In-Memory Layout
The BDFS resides entirely in a static memory buffer. It has a file table and a data region.

### 8.2. Features
- **Hierarchical Directories:** BDFS now supports a directory tree structure.
- **File Operations:** `bdfs_create_file`, `bdfs_delete_file`, `bdfs_read_file`, `bdfs_write_file`, `bdfs_rename_file`.
- **Directory Operations:** `bdfs_mkdir`, `bdfs_chdir`.
- **Colored Listings:** `bdfs_list_files` displays files and directories with different colors.

## 9. Shell (`shell/`)

The shell provides a command-line interface for interacting with the OS.

### Supported Commands:
- `help`: Displays a list of available commands.
- `clear`: Clears the console screen.
- `meminfo`: Shows PMM statistics.
- `time`: Displays the system uptime.
- `halt`, `reboot`, `shutdown`: System power commands.
- `ls`, `touch`, `rm`, `mv`, `mkdir`, `cd`: Filesystem commands.
- `write <file> <data>`: Writes data to a file.
- `cat <file>`: Displays the content of a file.
- `echo <text>`: Prints text to the screen.
- `cable <file>`: A simple text editor.
- `calc <expr>`: A simple calculator.
- `sysinfo`: Displays system information.
- `pulse`: Shows CPU and memory usage.
- `chrome`: Lists connected PCI devices.
- `applist`: Lists available applications.
- `*.bdx`: Executes BDX bytecode files.

## 10. Applications

BrainDance OS includes a few simple applications.

### 10.1. Cable Text Editor

`cable` is a simple, screen-oriented text editor. It provides basic text editing functionality, including:
-   Creating and opening files.
-   Editing text using the keyboard (insertion, deletion, backspace).
-   Saving files using `Ctrl+S`. This triggers a `write` syscall implemented via the BDX interpreter.
-   Quitting the editor using `Ctrl+Q`.
-   Navigating with the arrow keys.

### 10.2. Calculator

`calc` is a command-line calculator that evaluates mathematical expressions.
- It uses the **Shunting-yard algorithm** to convert the input infix expression into postfix notation.
- It then evaluates the postfix expression to get the result.
- Supports basic arithmetic operations (`+`, `-`, `*`, `/`) and parentheses.

## 11. BDX Executable Format

BrainDance OS uses a custom executable format called BDX. These are simple bytecode programs executed by the `execute_bdx` function. The BDX interpreter supports a small set of opcodes for basic operations like syscalls (e.g., writing a file) and exiting.

## 12. Build System (`Makefile`)

The `Makefile` automates the build process, compiling all C and assembly files, linking them into a kernel executable, and creating a bootable disk image (`bdos.img`).

## 13. System Layout

### 12.1. Memory Layout

| **Memory Location** | **Component** | **Description** |
| ------------------- | ------------- | --------------- |
| `0x1000`            | E820 Map      | Memory map from BIOS. |
| `0x7C00`            | Bootloader    | The first 512 bytes of the disk. |
| `0x8000`            | Kernel (Initial Load) | Temporary kernel location. |
| `0x90000`           | Paging Data   | Page Directory and Page Tables. |
| `0x100000` (1MB)    | Kernel        | Final location of the kernel. |
| `0x400000` (4MB)    | Kernel Heap   | Start of the kernel's dynamic memory. |

### 12.2. Disk Sector Layout

| **Sector(s)** | **Component** | **Description** |
| ------------- | ------------- | --------------- |
| 0             | Stage 1 Bootloader (BIOS Shell) | The Master Boot Record (MBR). |
| 1             | Stage 2 Bootloader (Kernel Loader) | The second stage of the bootloader. |
| 2 - 129       | Kernel        | The kernel binary. |