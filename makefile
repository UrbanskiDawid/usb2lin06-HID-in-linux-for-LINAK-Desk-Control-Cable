.DEFAULT_GOAL := default
help:
	@echo please use:
	@echo *all - to build all
	@echo *program - to build user-space examples
	@echo *kernel - to build kernel module

program:
	make -C EXAMPLES build
kernel:
	make -C KERNEL_MODULE build

default:	program
	make -C EXAMPLES run

debug:
	make -C EXAMPLES debug
	make -C EXAMPLES run
		
clean:
	make -C EXAMPLES clean
	make -C KERNEL_MODULE clean

all:	program	kernel
