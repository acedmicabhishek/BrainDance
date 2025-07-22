# BrainDance
An OS from Scratch aiming for cyberNet

Next Logical Step:
1. Basic Keyboard Input (Polling from PS/2 Controller)
Read key presses via port 0x60 and display them using print().

2. Simple Cursor Control
Move the cursor properly using I/O ports 0x3D4 and 0x3D5.


3. Basic Paging & Memory Management
Set up simple identity-mapped paging and page directories.

4. Basic Filesystem Driver (e.g., FAT12 from floppy image)
Let the kernel read a file from disk (this is a big one).