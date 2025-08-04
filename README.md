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

## PHASE 1: Bare-Metal Networking Foundation

### 1. PCI Device Scanning
- Build a PCI configuration space reader (ports `0xCF8`, `0xCFC`)
- Enumerate all PCI devices
- Detect network cards (Class: `0x02`, Subclass: `0x00`)

### 2. Choose a NIC
- **Recommended:** Intel 82540EM / E1000 (QEMU-supported)
- Alternatives: RTL8139, AMD PCnet

### 3. Write E1000 Driver
- Map MMIO registers from PCI BAR
- Setup descriptor rings for TX/RX
- Install IRQ handler for NIC
- Implement basic Ethernet frame send/receive
- Print frames via debug shell to verify

---

## PHASE 2: Protocol Stack

### 4. Ethernet Protocol
- Parse Ethernet headers
- Route frames by EtherType (`0x0800` = IPv4, `0x0806` = ARP)

### 5. ARP (Address Resolution Protocol)
- Build ARP cache (IP ↔ MAC)
- Handle ARP requests & send ARP replies

### 6. IPv4
- Parse and construct IP headers
- Route IP packets by protocol (ICMP, UDP, TCP)
- Support basic fragmentation (optional early)

### 7. ICMP
- Handle ICMP Echo Request & Reply (Ping)
- Add `ping <ip>` command to BD shell

---

## PHASE 3: Userland Networking Tools

### 8. Shell Commands
- `ifconfig` – show interface info
- `ping` – test ICMP
- `arp` – print ARP table

---

## PHASE 4: Transport Layer

### 9. UDP
- Implement basic UDP parsing & sending
- Build echo/receive tool for testing

### 10. TCP (Advanced)
- Handle 3-way handshake
- Implement TCP state machine
- Sliding window + ACKs
- Build `echo_server` or simple HTTP fetcher

---

## PHASE 5: Real Utility

### 11. DNS (UDP)
- Send DNS queries
- Parse responses
- Support A/AAAA record resolution

### 12. Apps for Johnny Silverhand
- `curl`-like app for HTTP
- Port scanner
- Reverse shell client/server
- File transfer via TCP

---

## BONUS: Cyberpunk Edition

- Packet sniffer (`tcpdump`-like)
- Raw socket interface
- Packet crafting/injector
- JohnnyNet custom protocol
- USB WiFi NIC driver (very advanced)

---

account check for git
test2