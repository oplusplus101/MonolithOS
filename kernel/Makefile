
OBJ			= obj/kernel.o \
			  obj/gdt.o \
			  obj/gdtasm.o \
			  obj/idt.o \
			  obj/idtasm.o \
			  obj/pci.o \
			  obj/ahci.o \
			  obj/screen.o \
			  obj/paging.o \
			  obj/bitmap.o \
			  obj/exceptions.o \
			  obj/drive.o
AS			= nasm
AS_PARAMS	= -f elf64
CC			= gcc
CC_PARAMS	= -c -ffreestanding -fno-stack-protector -fno-builtin -mno-red-zone -nostdlib -Wall -Werror -Wpedantic -Iinclude
KERNEL_EXEC = monolith.sys
LINKER		= linker.ld

$(KERNEL_EXEC): $(OBJ)
	ld $^ -o $(KERNEL_EXEC).tmp -T$(LINKER)
	objcopy -O pei-x86-64 $(KERNEL_EXEC).tmp $(KERNEL_EXEC)
	rm $(KERNEL_EXEC).tmp

obj/%.o: src/%.asm
	mkdir -p $(@D)
	$(AS) $< -o $@ $(AS_PARAMS)

obj/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) $< -o $@ $(CC_PARAMS)

clean:
	rm obj $(KERNEL_EXEC) -rf
