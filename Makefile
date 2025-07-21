all: bdos.img

BDbootloader.bin: BDbootloader.asm
	nasm -f bin BDbootloader.asm -o BDbootloader.bin

BDkernel.o: BDkernel.c
	i686-elf-gcc -m32 -ffreestanding -c BDkernel.c -o BDkernel.o

BDkernel.bin: BDkernel.o linker.ld
	i686-elf-ld -m elf_i386 -T linker.ld -o BDkernel.elf BDkernel.o
	objcopy -O binary BDkernel.elf BDkernel.bin

bdos.img: BDbootloader.bin BDkernel.bin
	cat BDbootloader.bin BDkernel.bin > bdos.img

run: bdos.img
	qemu-system-i386 -drive format=raw,file=bdos.img

clean:
	rm -f *.bin *.o *.elf bdos.img
