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

## 7. Shell (`shell/`)

The shell provides a command-line interface for interacting with the BrainDance OS. It allows users to execute various commands to query system information, manage the display, and perform basic system operations.

### Key Components:
- **`shell.h` / `shell.c`:** Defines the shell's interface and implements its core logic, including input handling, command parsing, and execution.
- **Command Buffer:** Stores user input up to `MAX_COMMAND_LENGTH`.
- **Command Handlers:** Dedicated functions for each supported command (e.g., `help_command`, `clear_command`).

### Supported Commands:
- `help`: Displays a list of available commands and their descriptions.
- `clear`: Clears the console screen.
- `meminfo`: Shows statistics about physical memory usage (total, used, free).
- `time`: Displays the system uptime in seconds.
- `halt`: Halts the CPU, requiring manual closure of the QEMU emulator.
- `ls`: Lists all files in the BDFS filesystem.
- `touch <filename>`: Creates a new, empty file.
- `rm <filename>`: Deletes a file.
- `write <filename> <data>`: Writes data to a file, overwriting any existing content.
- `cat <filename>`: Reads and displays the content of a file.
- `format`: Formats the disk, creating a new, empty BDFS filesystem.
- `ataread <lba>`: Reads a raw sector from the disk at the specified LBA.
- `atawrite <lba> <data>`: Writes raw data to a sector on the disk.

### Input Handling:
The shell continuously reads character input from the keyboard. It supports:
- **Typing:** Characters are echoed to the screen as they are typed.
- **Backspace:** Deletes the last character entered.
- **Enter:** Processes the current command in the buffer. The command is null-terminated, and the `process_command` function dispatches it to the appropriate handler.

## 8. Memory Management

BrainDance OS implements several layers of memory management to efficiently allocate and protect memory resources.

### 8.1. Paging (`memory/paging.c`, `include/paging.h`)
Paging is a memory management scheme that allows the operating system to store and retrieve data from secondary storage for use in main memory. It enables virtual memory, providing each process with its own isolated address space.

#### Key Components:
- **Page Directory:** A table containing 1024 entries, each pointing to a Page Table.
- **Page Table:** A table containing 1024 entries, each pointing to a physical page frame (4KB).
- **Page Directory Entry (PDE) Flags:**
    - `PDE_PRESENT`: Indicates if the page table is present in memory.
    - `PDE_RW`: Read/Write permission.
    - `PDE_USER`: User-mode access permission.
- **Page Table Entry (PTE) Flags:**
    - `PTE_PRESENT`: Indicates if the physical page is present in memory.
    - `PTE_RW`: Read/Write permission.
    - `PTE_USER`: User-mode access permission.

#### Functionality:
- `paging_install()`: Initializes the paging system, sets up the initial page directory and page table, identity maps the first 4MB of memory, and installs the page fault handler.
- `map_page(phys_addr, virt_addr, flags)`: Maps a given physical address to a virtual address with specified permissions. It creates new page tables if necessary.
- `unmap_page(virt_addr)`: Unmaps a virtual address, invalidating the corresponding Translation Lookaside Buffer (TLB) entry.
- `get_phys_addr(virt_addr)`: Retrieves the physical address corresponding to a given virtual address.
- **Page Fault Handler:** Catches memory access violations, prints debug information (faulting address, error code), and halts the system.

### 8.2. Heap (`memory/heap.c`, `include/heap.h`)
The heap provides dynamic memory allocation for the kernel. BrainDance OS currently uses a simple bump allocator for its kernel heap.

#### Key Components:
- `HEAP_START`: The starting virtual address of the heap (0x400000).
- `HEAP_END`: The ending virtual address of the heap (0x800000).
- `heap_ptr`: A pointer that tracks the next available address in the heap.

#### Functionality:
- `kmalloc(size)`: Allocates a block of `size` bytes from the heap. It simply increments `heap_ptr` and returns the previous value. Returns 0 if out of heap space.
- `kfree(ptr)`: A stub function for API compatibility. In a bump allocator, memory is not individually freed; it's typically reset or managed by higher-level allocators.

## 9. Drivers

This section details the hardware drivers implemented in BrainDance OS.

### 9.1. Keyboard Driver (`drivers/keyboard_driver.c`, `include/keyboard.h`)
The keyboard driver handles input from the PS/2 keyboard, converting scancodes into ASCII characters.

#### Key Components:
- `KBD_DATA_PORT` (0x60): Port for reading keyboard data.
- `KBD_STATUS_PORT` (0x64): Port for reading keyboard status.
- `kbd_us[128]`: A scancode-to-ASCII mapping table for a basic US keyboard layout.
- `last_char`: A global variable used to store the last character received from the keyboard interrupt handler, making it available for polling by functions like `keyboard_get_char()`.

#### Functionality:
- `keyboard_install()`: Installs the keyboard interrupt handler (IRQ1) to process keyboard events.
- `keyboard_handler(regs_t *r)`: The interrupt service routine for the keyboard. It reads the scancode from the data port, checks for key presses (ignoring releases for now), converts the scancode to an ASCII character using `kbd_us`, and stores the character in `last_char`.
- `keyboard_get_char()`: A blocking function that waits until a character is available in `last_char`, retrieves it, clears `last_char`, and returns the character. This function is used by the shell to get user input.

### 9.2. ATA Driver (`drivers/ata/ata.c`, `include/ata.h`)
The ATA (Advanced Technology Attachment) driver provides an interface for reading from and writing to IDE hard drives. It communicates with the drive controller using I/O ports.

#### Key Components:
- **I/O Ports:** Defines the standard port addresses for the primary ATA bus (e.g., `ATA_DATA_PORT`, `ATA_STATUS_CMD_PORT`).
- **Commands:** Defines ATA commands like `ATA_CMD_READ_SECTORS` and `ATA_CMD_WRITE_SECTORS`.
- **Status Bits:** Defines the bits in the status register (e.g., `ATA_SR_BSY`, `ATA_SR_DRDY`, `ATA_SR_ERR`).

#### Functionality:
- `ata_init()`: Initializes the ATA driver. It performs a software reset on the drive to ensure it's in a known state, then selects the master drive on the primary bus.
- `ata_read_sector(lba, buffer)`: Reads a single 512-byte sector from the drive at the specified Logical Block Address (LBA) into the provided buffer.
- `ata_write_sector(lba, buffer)`: Writes a single 512-byte sector from the buffer to the drive at the specified LBA.
- `ata_poll()`: A helper function that waits for the drive to be ready by polling the status port until the `BSY` (Busy) bit is clear.

## 10. Filesystem (BDFS)

BrainDance OS includes a simple, custom-built filesystem called BDFS (BrainDance File System). It is designed to be easy to implement and understand, providing basic file operations like creation, deletion, reading, and writing.

### 10.1. On-Disk Layout

The BDFS has a straightforward on-disk structure:
- **File Table:** A fixed-size table that stores metadata for all files. It starts at sector `BDFS_FILE_TABLE_SECTOR_START` (currently 129, to avoid overwriting the kernel) and occupies `BDFS_FILE_TABLE_SECTORS` (4).
    - The first 4 bytes of the file table contain the magic number `0x42444653` ("BDFS") to identify the filesystem.
- **Data Region:** The rest of the disk, starting at `BDFS_DATA_SECTOR_START`, is used for storing file data.

### 10.2. File Entry (`bdfs_file_entry_t`)

Each entry in the file table is a `bdfs_file_entry_t` structure, which contains:
- `name`: The filename (up to 16 characters).
- `start_sector`: The starting LBA of the file's data. A value of `(uint32_t)-1` indicates that no sectors have been allocated yet.
- `length`: The length of the file in bytes.

### 10.3. Filesystem Operations (`fs/bdfs.c`, `include/bdfs.h`)

- `bdfs_init()`: Initializes the filesystem. It checks for the BDFS magic number. If it's not found, it creates a new, empty filesystem by formatting the file table. Otherwise, it loads the existing file table from disk.
- `bdfs_create_file(filename)`: Creates a new, empty file with the given name.
- `bdfs_delete_file(filename)`: Deletes a file by clearing its entry in the file table. Note that the data blocks are not overwritten, only marked as available.
- `bdfs_read_file(filename, buffer, ...)`: Reads the entire content of a file into the provided buffer.
- `bdfs_write_file(filename, buffer, ...)`: Writes data to a file. If the file is new, it allocates a contiguous block of sectors. If the file exists, it overwrites the existing data. BDFS does not support growing files; you must write the entire file content at once.
- `bdfs_list_files()`: Lists all files in the filesystem.
- `bdfs_stat_file(filename, entry)`: Retrieves metadata (the file entry) for a given file.

## 11. System Layout

This section provides a map of how BrainDance OS is laid out in memory and on the disk.

### 11.1. Memory Layout

| **Memory Location** | **Component** | **Description** |
| ------------------- | ------------- | --------------- |
| `0x7C00` | Bootloader | The first 512 bytes of the disk, loaded by the BIOS. |
| `0x8000` | Kernel (Initial Load) | The kernel is temporarily loaded here before being moved to its final location. |
| `0x100000` (1MB) | Kernel | The final location of the kernel code, data, and BSS, as defined by the linker script. |
| `0x400000` (4MB) | Kernel Heap | The start of the kernel's dynamic memory allocation area. |

### 11.2. Disk Sector Layout

| **Sector(s)** | **Component** | **Description** |
| ------------- | ------------- | --------------- |
| 0 | Bootloader | The Master Boot Record (MBR). |
| 1 - 128 | Kernel | The kernel binary, loaded by the bootloader. |
| 129 - 132 | BDFS File Table | The metadata for the BrainDance File System. |
| 133+ | BDFS Data | The region where file content is stored. |