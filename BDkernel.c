extern char _bss_start;
extern char _bss_end;
extern char _stack_start;
extern char _stack_end;

void kernel_main() {
    clear_screen(0x07);

    print("BrainDance OS Booted Successfully\n\n", 0x0A);

    log("INFO", "System initialized");
    print("Memory Info:\n", 0x07);

    print("  BSS Size: ", 0x07);
    print_int(&_bss_end - &_bss_start, 0x07);
    print(" bytes\n", 0x07);

    print("  Stack Size: ", 0x07);
    print_int(&_stack_end - &_stack_start, 0x07);
    print(" bytes\n", 0x07);

    kprintf("Hello %s!\n", "Abheesh");
    kprintf("Value: %d, Address: %x\n", 1337, 0xB8000);

}
