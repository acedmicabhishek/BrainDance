# BrainDance
An OS from Scratch aiming for cyberNet

## done : 

IDT structure

ISRs (0–31)

Basic IRQ remapping (if going for hardware)

Use kprintf() inside ISRs

# What to implement:

## 2. Timer (PIT) Initialization
Enable regular ticks via IRQ0 (for scheduling later).

PIT init code (channel 0, mode 2, freq = 100 Hz)

Tick counter inside timer_handler()

Use it to test log() or scroll logic

## 3. Keyboard Driver
Handle keyboard interrupts via IRQ1.


Read scancode from 0x60

Convert to ASCII (basic US layout)

Echo input on screen

## 4. Physical Memory Management (PMM)
You already track _bss_end → now:

Build a simple bump allocator

Or better: a bitmap-based page allocator (like alloc_frame() / free_frame())

This unlocks:

Paging

Heap

Multitasking later

## 5. Paging (Virtual Memory)
Enable 4KB page mapping (x86 paging):

Set up Page Directory + Tables

Identity-map first few MBs

Enable bit in CR0, load CR3

## 6. Basic Kernel Shell / Monitor
Let’s make BrainDance talk to you:

Take keyboard input

Parse basic commands like:

meminfo

help

uptime

clear

## 7. Heap & Dynamic Allocation
Implement:

kmalloc(), kfree()

Simple malloc-style allocator (bump or linked-list)
This helps build file systems, drivers, etc.