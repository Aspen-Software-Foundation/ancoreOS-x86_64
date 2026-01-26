# Copyright (C) 2026 Aspen Software Foundation

# 	Module: Makefile	
# 	Description: The Makefile for the VNiX Operating System.
# 	Author: Yazin Tantawi (and Jerry Jhird)

# 	All components of the VNiX Operating System, except where otherwise noted,
# 	are copyright of the Aspen Software Foundation (and the corresponding author(s)) and licensed under GPLv2 or later.
# 	For more information on the GNU Public License Version 2, please refer to the LICENSE file 
# 	or to the link provided here: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

# THIS OPERATING SYSTEM IS PROVIDED "AS IS" AND "AS AVAILABLE" UNDER
# THE GNU GENERAL PUBLIC LICENSE VERSION 2, WITHOUT
# WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, TITLE, AND NON-INFRINGEMENT.

# TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
# THE AUTHORS, COPYRIGHT HOLDERS, OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE), ARISING IN ANY WAY OUT OF THE USE OF THIS OPERATING SYSTEM,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE OPERATING SYSTEM IS
# WITH YOU. SHOULD THE OPERATING SYSTEM PROVE DEFECTIVE, YOU ASSUME THE COST OF
# ALL NECESSARY SERVICING, REPAIR, OR CORRECTION.

# YOU SHOULD HAVE RECEIVED A COPY OF THE GNU GENERAL PUBLIC LICENSE
# ALONG WITH THIS OPERATING SYSTEM; IF NOT, WRITE TO THE FREE SOFTWARE
# FOUNDATION, INC., 51 FRANKLIN STREET, FIFTH FLOOR, BOSTON,
# MA 02110-1301, USA.

CFLAGS := -I src/ -ffreestanding -Wall -Wextra -Wunused-parameter -static -nostartfiles -nostdlib -fno-pie -no-pie -mno-red-zone -mcmodel=large -T linker.ld -I src/includes/ -I src/includes/klibc/ -I src/drivers/memory/liballoc/ -D_ALLOC_SKIP_DEFINE
LDFLAGS := -nostdlib -static -z noexecstack
QEMU_CPU ?=
QEMU_MEM ?= -m 1G

all: clean kernel build/uefi-usb.img 

kernel:
	mkdir -p build
	@echo "Compiling kernel and drivers..."
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
	gcc -ffreestanding -c src/drivers/memory/liballoc/liballoc.c -o build/liballoc.o $(CFLAGS) 
	gcc -ffreestanding -c src/drivers/memory/liballoc-impl.c -o build/liballoc-impl.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/memory/vmm.c -o build/vmm.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/util/log-info.c -o build/log-info.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/util/math.c -o build/math.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/pic/pic.c -o build/pic.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/pic/pic_irq.c -o build/pic_irq.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/pic/apic/apic.c -o build/apic.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/pic/apic/apic_irq.c -o build/apic_irq.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/shell/keyboard.c -o build/keyboard.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/storage/storage.c -o build/storage.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/storage/sata.c -o build/storage.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/shell/shell.c -o build/shell.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/time/time.c -o build/time.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/storage/ata.c -o build/ata.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/storage/stinit.c -o build/stinit.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/storage/atapi.c -o build/atapi.o $(CFLAGS)
	nasm -f elf64 src/arch/x86_64/isr_stubs.asm -o build/isr_stubs.o

# After much research, i've concluded on this linking order because it looks much better than the hellish alternative i initially had
	ld $(LDFLAGS) -T linker.ld -o build/kernel.elf \
		build/kernel.o \
		build/term.o \
		build/stdio.o \
		build/stdlib.o \
		build/serial.o \
		build/io.o \
		build/idt.o \
		build/gdt.o \
		build/pmm.o \
		build/isr.o \
		build/isrs_gen.o \
		build/isr_stubs.o \
		build/string.o \
		build/liballoc.o \
		build/liballoc-impl.o \
		build/vmm.o\
		build/log-info.o \
		build/math.o \
		build/pic.o \
		build/pic_irq.o \
		build/apic.o \
		build/apic_irq.o \
		build/keyboard.o \
		build/shell.o \
		build/storage.o \
		build/time.o \
		build/ata.o\
		build/atapi.o \
		build/stinit.o
	objcopy --strip-debug build/kernel.elf


build/uefi-usb.img: kernel
	@echo "Creating raw UEFI image..."
	
	rm -rf build/usb_root
	mkdir -p build/usb_root/EFI/BOOT build/usb_root/boot
	
	cp limine/BOOTX64.EFI build/usb_root/EFI/BOOT/
	cp build/kernel.elf build/usb_root/boot/
	cp Assets/wallpaper4k.png build/usb_root/boot/ 2>/dev/null || true
	cp src/limine.conf build/usb_root/
	
	echo "fs0:" > build/usb_root/startup.nsh
	echo "cd \\EFI\\BOOT" >> build/usb_root/startup.nsh
	echo "BOOTX64.EFI" >> build/usb_root/startup.nsh
	
	@echo "Creating FAT32 image..."
	dd if=/dev/zero of=build/VNiX-uefi_dev-prototype.img bs=1080K count=64
	mkfs.fat -F 32 build/VNiX-uefi_dev-prototype.img 2>/dev/null || sudo mkfs.fat -F 32 build/VNiX-uefi_dev-prototype.img

# I decided to add a little interactive box because why not?
	@echo "Copying files to image..."
	mcopy -i build/VNiX-uefi_dev-prototype.img -s build/usb_root/* ::
	@echo ""
	@echo "IMAGE CREATED at: build/VNiX-uefi_dev-prototype.img"
	@echo ""
	@echo "TO WRITE TO USB DRIVE:"
	@echo "sudo dd if=build/VNiX-uefi_dev-prototype.img of=/dev/sdX bs=4M status=progress"
	@echo "Or use a tool like balenaEtcher/Rufus to write the image onto a USB drive."
	@echo ""
	@echo "Replace /dev/sdX with your USB device (e.g: /dev/sdb)"
	@echo "Check with: lsblk or sudo fdisk -l"
	@echo ""
	@echo "IMPORTANT: After writing, you MUST:"
	@echo "1. Go to BIOS/UEFI settings"
	@echo "2. Disable Secure Boot"
	@echo "3. Set USB as first boot device "
	@echo "   (or press F9 for the boot menu if your machine supports it)"
	@echo ""
	@echo "Alternatively, you can run 'make run' to test the UEFI raw image."
	@echo "================================================"
	@echo "Note: this image will NOT work for Non-UEFI/Legacy BIOS systems."
	@echo ""

run:
# I think all devs know this by now but qemu can differ from real hardware, so dont rely on it 100%
	qemu-system-x86_64 $(if $(QEMU_CPU),-cpu $(QEMU_CPU)) \
		$(QEMU_MEM) \
		-bios ovmf/OVMF.fd \
		-drive file=build/VNiX-uefi_dev-prototype.img,format=raw \
		-serial stdio

clean:
	rm -rf build


# generate compile_commands.json for clangd
#cc_commands:
#	bear -- make

# ^^^ This kept looping the build so i disabled it for now. The file is also deleted so dont even bother.