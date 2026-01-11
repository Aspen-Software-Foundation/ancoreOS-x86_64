# 	Copyright (C) 2026 Aspen Software Foundation
#
# 	Module: Makefile
# 	Description: The main build system for the VNiX Operating System
# 	Author: Yazin Tantawi (and Jerry Jhird)
#
# All components of the VNiX Operating System, except where otherwise noted, 
# are copyright of the Aspen Software Foundation (and the corresponding author(s)) and licensed under GPLv2 or later.
# For more information on the Gnu Public License Version 2, please refer to the LICENSE file
# or to the link provided here: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
#
# THIS OPERATING SYSTEM IS PROVIDED "AS IS" AND "AS AVAILABLE" UNDER 
# THE GNU GENERAL PUBLIC LICENSE VERSION 2, WITHOUT
# WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, TITLE, AND NON-INFRINGEMENT.
# 
# TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
# THE AUTHORS, COPYRIGHT HOLDERS, OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE), ARISING IN ANY WAY OUT OF THE USE OF THIS OPERATING SYSTEM,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE OPERATING SYSTEM IS
# WITH YOU. SHOULD THE OPERATING SYSTEM PROVE DEFECTIVE, YOU ASSUME THE COST OF
# ALL NECESSARY SERVICING, REPAIR, OR CORRECTION.
#
# YOU SHOULD HAVE RECEIVED A COPY OF THE GNU GENERAL PUBLIC LICENSE
# ALONG WITH THIS OPERATING SYSTEM; IF NOT, WRITE TO THE FREE SOFTWARE
# FOUNDATION, INC., 51 FRANKLIN STREET, FIFTH FLOOR, BOSTON,
# MA 02110-1301, USA.


CFLAGS := -ffreestanding -Wall -Wextra -Wunused-parameter -static -nostartfiles -nostdlib -fno-pie -no-pie -mno-red-zone -mcmodel=large -T linker.ld
QEMU_CPU ?=
QEMU_MEM ?= -m 2G
all: clean build/uefi.img kernel run

kernel:
	mkdir -p build
	gcc -ffreestanding -c src/kernel/kernel.c -o build/kernel.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/framebuffer/src/term.c -o build/term.o $(CFLAGS)
	gcc -ffreestanding -c src/drivers/klibc/memory.c -o build/memory.o $(CFLAGS) 
	gcc -ffreestanding -c src/drivers/klibc/stdio.c -o build/stdio.o $(CFLAGS)  
	gcc -ffreestanding -c src/drivers/util/serial.c -o build/serial.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/gdt_idt.c -o build/gdt_idt.o $(CFLAGS)
	gcc -ffreestanding -c src/arch/x86_64/io.c -o build/io.o $(CFLAGS) 
	ld -T linker.ld -nostdlib -static -o build/kernel.elf build/kernel.o build/term.o build/stdio.o build/memory.o  build/serial.o build/gdt_idt.o build/io.o

# Author: Jerry Jhird
#License: MPLv2.0
#Source: https://github.com/jerryjhird/CuoreOS/blob/master/Makefile
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
# End Code Attribution

clean:
	rm -rf build

run:
	qemu-system-x86_64 $(if $(QEMU_CPU),-cpu $(QEMU_CPU)) \
		$(QEMU_MEM) \
		-bios ovmf/OVMF.fd \
		-drive if=virtio,format=raw,file=build/uefi.img \
		-serial stdio \


