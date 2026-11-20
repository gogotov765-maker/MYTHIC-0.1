CC      = gcc
LD      = ld
CFLAGS  = -m32 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -O2 -Wall -Wextra -nostdlib
LDFLAGS = -m elf_i386 -T src/linker.ld -nostdlib

all: mythic.bin

boot.o: src/boot.s
	$(CC) -m32 -c $< -o $@

kernel.o: src/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

mythic.bin: boot.o kernel.o
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o

iso: mythic.bin
	cp mythic.bin iso/boot/mythic.bin
	grub-mkrescue -o mythic.iso iso

run: mythic.bin
	qemu-system-i386 -kernel mythic.bin

run-iso: iso
	qemu-system-i386 -cdrom mythic.iso

clean:
	rm -f *.o mythic.bin mythic.iso iso/boot/mythic.bin
