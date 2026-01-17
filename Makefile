# 	Copyright (C) 2026 Aspen Software Foundation
#
# 	Module: Makefile
# 	Description: The main build system for the Ancore Operating System
# 	Author: Yazin Tantawi (and Jerry Jhird)

# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at 
# https://mozilla.org/MPL/2.0/.

CFLAGS := -I src/ -ffreestanding -Wall -Wextra -Wunused-parameter -static -nostartfiles -nostdlib -fno-pie -no-pie -mno-red-zone -mcmodel=large -T linker.ld -I src/includes/ -I src/includes/klibc/ 
QEMU_CPU ?=
QEMU_MEM ?= -m 2G

all: clean build/uefi.img kernel run

kernel:

#I dont care if this is incorrect, if it works, it works.
	mkdir -p build
	gcc -ffreestanding -c src/kernel/kernel.c -o build/kernel.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/terminal/src/term.c -o build/term.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/klibc/stdlib.c -o build/stdlib.o $(CFLAGS) 
	gcc -ffreestanding -c src/drivers/klibc/stdio.c -o build/stdio.o $(CFLAGS) 
	gcc -ffreestanding -c src/drivers/klibc/string.c -o build/string.o $(CFLAGS)  
	gcc -ffreestanding -c src/drivers/util/serial.c -o build/serial.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/gdt.c -o build/gdt.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/idt.c -o build/idt.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/io.c -o build/io.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/memory/pmm.c -o build/pmm.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/isr.c -o build/isr.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/isrs_gen.c -o build/isrs_gen.o $(CFLAGS)
	nasm -f elf64 src/arch/x86_64/isr_stubs.asm -o build/isr_stubs.o

	ld -T linker.ld -nostdlib -static -o build/kernel.elf build/kernel.o build/term.o build/stdio.o build/stdlib.o  build/serial.o  build/io.o build/idt.o build/gdt.o build/pmm.o build/isr.o build/isrs_gen.o build/isr_stubs.o build/string.o
build/uefi.img: kernel 
	@printf ">>> Creating UEFI bootable image: $@\n"

	dd if=/dev/zero of=build/uefi.img bs=1M count=64

	parted -s build/uefi.img mklabel gpt
	parted -s build/uefi.img mkpart ESP fat32 1MiB 100%
	parted -s build/uefi.img set 1 esp on

	mformat -i build/uefi.img ::/

	mmd -i build/uefi.img ::EFI
	mmd -i build/uefi.img ::EFI/BOOT
	mcopy -i build/uefi.img limine/BOOTX64.EFI ::EFI/BOOT/
	mmd -i build/uefi.img ::boot
	mcopy -i build/uefi.img build/kernel.elf ::boot/
	mcopy -i build/uefi.img src/limine.conf ::
	mcopy -i build/uefi.img Assets/wallpaper4k.png ::boot/

run:
	qemu-system-x86_64 $(if $(QEMU_CPU),-cpu $(QEMU_CPU)) \
		$(QEMU_MEM) \
		-bios ovmf/OVMF.fd \
		-drive if=virtio,format=raw,file=build/uefi.img \
		-serial stdio \

clean:
	rm -rf build

# generate compile_commands.json for clangd
cc_commands:
	bear -- make