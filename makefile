.DEFAULT_GOAL := program

help:
	@echo please use:
	@echo *all - to build all
	@echo *program - to build user-space examples
	@echo *kernel - to build kernel module

program:
	make -C EXAMPLES build
kernel:
	make -C KERNEL_MODULE build
all:	program
