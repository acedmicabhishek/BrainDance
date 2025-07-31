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

# 📦 Upcoming Features / Modules

## 1. Built-in Apps Folder

```
/app/
├── core/    → echo, sysinfo, memstat, etc.
├── utils/   → textedit.bd (nano-style), convert.bd, scan.bd
├── games/   → netrunner.bd, deckcrack.bd, breakchip.bd
├── ai/      → analyze.bd, ghostsync.bd (flair-only)
```

## 2. Basic Text Editor

- Nano-style built-in editor ( C.A.B.L.E. – Cybernetic Access Buffer for Lightweight Editing ) 
- Save/load to BDFS files  
- Stored in `/app/utils/cable.bd`  

## 3. Program Execution Framework

- Load `.bdx` files from `/soul/`  
- Simple interpreter or native loader  
- CLI tools & apps can be run from shell  

## 4. System Call Interface (Mini ABI)

System-level functions exposed to programs:

- `syscall 0` → print  
- `syscall 1` → read  
- `syscall 2` → open file  
- `syscall 3` → malloc (optional)  

Helps programs interact with BD kernel  

## 5. Fake AI Hooks (Flair Only)

- Monitor `/drift/`, `/ghost/`, etc.  
- Print fake logs like:  

```
[brain] AI analyzing drift logs...
[brain] Ghost sync triggered from ghost/420.tmp
```

## 6. Optional: Networking Simulation

- Fake sockets in `/vault/netrunner.sock`  
- Could simulate ping, connect, hack, etc.  
- Used in mini-game or CLI simulation  

---

##  Intentionally Skipped / Not Needed

- File permissions system  
- Mountable virtual folders  
- Interactive shell with autocomplete  
- Scriptable init system  
- Fancy Cyberdeck ASCII UI shell  
- Icons for files/folders  