
BOOTLOADER_EXEC	= bootloader/bootloader.efi
KERNEL_EXEC		= kernel/monolith.sys
OVMFDIR			= ./OVMF
IMG				= Monolith.img

CC			= gcc
LD			= ld
OBJCOPY		= objcopy
AS			= nasm
CC_PARAMS	= -m64 -c -std=c2x -O0 -nostdlib -ffreestanding -fno-builtin -fno-stack-protector \
			  -fno-stack-check -fno-exceptions -mno-stack-arg-probe -mno-red-zone -Iinclude -Wall -Wpedantic -Werror
QEMU		= qemu-system-x86_64
QEMU_PARAMS = -m 1G -cpu qemu64 -monitor stdio \
			  -drive if=pflash,format=raw,unit=0,file="$(OVMFDIR)/OVMF_CODE.fd",readonly=on \
			  -drive if=pflash,format=raw,unit=1,file="$(OVMFDIR)/OVMF_VARS.fd" -net none \
			  -drive id=disk,file=$(IMG),format=raw

run: $(IMG)
	$(QEMU) $(QEMU_PARAMS)

$(IMG): $(BOOTLOADER_EXEC) $(KERNEL_EXEC)
	dd if=/dev/zero of=$(IMG) bs=1k count=1440
	mformat -i $(IMG) -f 1440 ::
	mmd   -i $(IMG) ::/EFI
	mmd   -i $(IMG) ::/EFI/BOOT
	mcopy -i $(IMG) $(BOOTLOADER_EXEC) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $(IMG) $(KERNEL_EXEC) ::/MONOLITH.SYS
	mcopy -i $(IMG) startup.nsh ::/STARTUP.NSH

debug: $(IMG)
	$(QEMU) -gdb tcp::9000 $(QEMU_PARAMS)


$(BOOTLOADER_EXEC): bootloader/bootloader.c
	cd bootloader && make

$(KERNEL_EXEC):
	cd kernel && make

clean:
	cd kernel && make clean
	rm $(BOOTLOADER_EXEC) -rf
