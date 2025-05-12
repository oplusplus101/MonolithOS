
OBJ			= obj/kernel.o
CC			= gcc
CC_PARAMS	= -c -ffreestanding -fno-stack-protector -fno-builtin -mno-red-zone -nostdlib -Wall -Werror -Wpedantic -Iinclude
KERNEL_EXEC = monolith.sys
LINKER		= linker.ld

$(KERNEL_EXEC): $(OBJ)
	ld $^ -o $(KERNEL_EXEC).tmp -T$(LINKER)
	objcopy -O pei-x86-64 $(KERNEL_EXEC).tmp $(KERNEL_EXEC)
	rm $(KERNEL_EXEC).tmp

obj/%.o: src/%.c
	mkdir -p $(@D)
	gcc $< -o $@ $(CC_PARAMS)

clean:
	rm obj $(KERNEL_EXEC) -rf
