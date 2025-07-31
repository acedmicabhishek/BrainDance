CFLAGS = -m32 -ffreestanding -I.
all: bdos.img

# Compile bootloader
BDbootloader.bin: boot/BDbootloader.asm
	nasm -f bin boot/BDbootloader.asm -o BDbootloader.bin

# Compile libc
libc/memcore.o: libc/memcore.c include/memcore.h
	i686-elf-gcc $(CFLAGS) -c libc/memcore.c -o libc/memcore.o

# Compile PMM
memory/pmm.o: memory/pmm.c include/pmm.h
	i686-elf-gcc $(CFLAGS) -c memory/pmm.c -o memory/pmm.o

# Compile Paging
memory/paging.o: memory/paging.c include/paging.h
	i686-elf-gcc $(CFLAGS) -c memory/paging.c -o memory/paging.o

# Compile Heap
memory/heap.o: memory/heap.c include/heap.h
	i686-elf-gcc $(CFLAGS) -c memory/heap.c -o memory/heap.o

# Compile architecture-specific files
arch/i386/idt.o: arch/i386/idt.c include/idt.h
	i686-elf-gcc $(CFLAGS) -c arch/i386/idt.c -o arch/i386/idt.o

arch/i386/isr.o: arch/i386/isr.c include/isr.h include/idt.h
	i686-elf-gcc $(CFLAGS) -c arch/i386/isr.c -o arch/i386/isr.o

arch/i386/isr_asm.o: arch/i386/isr.asm
	nasm -f elf32 arch/i386/isr.asm -o arch/i386/isr_asm.o

arch/i386/irq_asm.o: arch/i386/irq.asm
	nasm -f elf32 arch/i386/irq.asm -o arch/i386/irq_asm.o

arch/i386/load_idt.o: arch/i386/load_idt.asm
	nasm -f elf32 arch/i386/load_idt.asm -o arch/i386/load_idt.o

arch/i386/pic.o: arch/i386/pic.c include/pic.h include/ports.h
	i686-elf-gcc $(CFLAGS) -c arch/i386/pic.c -o arch/i386/pic.o

arch/i386/irq.o: arch/i386/irq.c include/irq.h include/idt.h include/pic.h include/regs.h
	i686-elf-gcc $(CFLAGS) -c arch/i386/irq.c -o arch/i386/irq.o

arch/i386/timer.o: arch/i386/timer.c include/timer.h include/irq.h include/ports.h include/memcore.h
	i686-elf-gcc $(CFLAGS) -c arch/i386/timer.c -o arch/i386/timer.o

drivers/keyboard_driver.o: drivers/keyboard_driver.c include/keyboard.h include/ports.h include/irq.h include/memcore.h
	i686-elf-gcc $(CFLAGS) -c drivers/keyboard_driver.c -o drivers/keyboard_driver.o

# Compile ATA driver
drivers/ata/ata.o: drivers/ata/ata.c include/ata.h include/ports.h
	i686-elf-gcc $(CFLAGS) -c drivers/ata/ata.c -o drivers/ata/ata.o

# Compile Shell
shell/shell.o: shell/shell.c include/shell.h
	i686-elf-gcc $(CFLAGS) -c shell/shell.c -o shell/shell.o

# Compile BDFS
fs/bdfs.o: fs/bdfs.c include/bdfs.h
	i686-elf-gcc $(CFLAGS) -c fs/bdfs.c -o fs/bdfs.o

# Compile apps
app/utils/cable.o: app/utils/cable.c include/cable.h
	i686-elf-gcc $(CFLAGS) -c app/utils/cable.c -o app/utils/cable.o

# Compile exec
exec/exec.o: exec/exec.c include/exec.h
	i686-elf-gcc $(CFLAGS) -c exec/exec.c -o exec/exec.o

# Compile kernel
kernel/BDkernel.o: kernel/BDkernel.c include/memcore.h include/idt.h include/isr.h include/keyboard.h
	i686-elf-gcc $(CFLAGS) -c kernel/BDkernel.c -o kernel/BDkernel.o

# Link kernel
BDkernel.bin: kernel/BDkernel.o libc/memcore.o memory/pmm.o memory/paging.o memory/heap.o arch/i386/idt.o arch/i386/isr.o arch/i386/isr_asm.o arch/i386/load_idt.o arch/i386/pic.o arch/i386/irq.o arch/i386/irq_asm.o arch/i386/timer.o drivers/keyboard_driver.o drivers/ata/ata.o shell/shell.o fs/bdfs.o app/utils/cable.o exec/exec.o kernel/linker.ld
	i686-elf-ld -m elf_i386 -T kernel/linker.ld -o BDkernel.elf kernel/BDkernel.o libc/memcore.o memory/pmm.o memory/paging.o memory/heap.o arch/i386/idt.o arch/i386/isr.o arch/i386/isr_asm.o arch/i386/load_idt.o arch/i386/pic.o arch/i386/irq.o arch/i386/irq_asm.o arch/i386/timer.o drivers/keyboard_driver.o drivers/ata/ata.o shell/shell.o fs/bdfs.o app/utils/cable.o exec/exec.o
	objcopy -O binary BDkernel.elf BDkernel.bin

# Create bootable image
bdos.img: BDbootloader.bin BDkernel.bin
	dd if=/dev/zero of=bdos.img bs=512 count=130
	dd if=BDbootloader.bin of=bdos.img conv=notrunc
	dd if=BDkernel.bin of=bdos.img seek=1 conv=notrunc

# Run with QEMU
run: bdos.img
	qemu-system-i386 -drive format=raw,file=bdos.img

# Clean build files
clean:
	rm -f *.bin *.o *.elf bdos.img boot/*.bin kernel/*.o kernel/*.elf libc/*.o arch/i386/*.o drivers/*.o drivers/ata/*.o fs/*.o app/utils/*.o exec/*.o
