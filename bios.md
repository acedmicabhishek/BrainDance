# BrainDance BIOS

The BrainDance Operating System now features a custom, two-stage bootloader that begins with a lightweight, interactive BIOS shell.

## Boot Sequence

1.  **Silent Countdown:** Upon starting the machine, a silent 3-second countdown begins.
    -   If no key is pressed, the system automatically proceeds to load the second-stage bootloader and start the OS.
    -   If the user presses the `2` key during the countdown, the system enters the BIOS shell.

2.  **BIOS Shell:** This is a minimal, interactive shell for basic pre-boot operations.
    -   Upon entry, it automatically displays the current VGA video mode information.
    -   It provides a command prompt for user input.

### Supported Commands

-   `start`: Exits the BIOS shell and proceeds to boot the operating system by loading the second-stage bootloader.
-   `res`: (Reserved) A placeholder for future resolution-setting functionality. Currently prints "n/a".

## Technical Details

-   **File:** [`boot/bios.asm`](boot/bios.asm)
-   **Mode:** 16-bit Real Mode
-   **Size:** The entire first-stage bootloader, including the BIOS shell, is contained within a single 512-byte boot sector.