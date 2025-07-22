all: bdos.img

# Compile bootloader
BDbootloader.bin: BDbootloader.asm
	nasm -f bin BDbootloader.asm -o BDbootloader.bin

# Compile memory core library
lib/memcore.o: lib/memcore.c lib/memcore.h
	i686-elf-gcc -m32 -ffreestanding -c lib/memcore.c -o lib/memcore.o

# Compile kernel with memcore
BDkernel.o: BDkernel.c lib/memcore.o
	i686-elf-gcc -m32 -ffreestanding -c BDkernel.c -o BDkernel.o

# Link kernel
BDkernel.bin: BDkernel.o lib/memcore.o linker.ld
	i686-elf-ld -m elf_i386 -T linker.ld -o BDkernel.elf BDkernel.o lib/memcore.o
	objcopy -O binary BDkernel.elf BDkernel.bin

# Create bootable image
bdos.img: BDbootloader.bin BDkernel.bin
	cat BDbootloader.bin BDkernel.bin > bdos.img

# Run with QEMU
run: bdos.img
	qemu-system-i386 -drive format=raw,file=bdos.img

# Clean build files
clean:
	rm -f *.bin *.o *.elf bdos.img lib/*.o
