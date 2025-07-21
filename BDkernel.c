void kernel_main() {
    const char* msg = "Welcome to brainDance Kernel!";
    char* vga = (char*)0xB8000;

    for (int i = 0; msg[i] != 0; ++i) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x0F;
    }

    while (1) {}
}
