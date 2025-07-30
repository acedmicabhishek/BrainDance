# BrainDance
An OS from Scratch aiming for cyberNet.

BrainDance is an educational operating system built from scratch. This project documents the journey of creating a simple OS, covering its architecture, components, and functionality from the bootloader to a basic shell.

## Features

*   **Bootloader**: A 16-bit assembly bootloader that transitions the CPU to 32-bit protected mode.
*   **Kernel**: A C-based kernel that initializes core systems like the IDT, ISRs, IRQs, and a PIT timer.
*   **Memory Management**: Includes both paging for virtual memory and a simple bump allocator for kernel heap management.
*   **Drivers**: Supports PS/2 keyboard input and ATA (IDE) hard drive for storage.
*   **Filesystem**: A custom-built filesystem, BDFS (BrainDance File System), with support for creating, deleting, reading, and writing files.
*   **Shell**: An interactive command-line interface with commands for file operations, system information (`meminfo`, `time`), and direct hardware access (`ataread`, `atawrite`).
*   **Standard Library**: A minimal `libc` implementation providing essential functions like `memcpy`, `strlen`, and a `kprintf` for formatted output.

## Building and Running

The project uses a `Makefile` to automate the build process.

1.  **Build the OS:**
    ```sh
    make
    ```
    This will produce a bootable disk image named `bdos.img`.

2.  **Run with QEMU:**
    ```sh
    qemu-system-i386 -hda bdos.img
    ```

## Documentation

For a complete and detailed explanation of the OS architecture, components, and functionality, please see the full documentation in [`docs.md`](docs.md).
