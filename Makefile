.DEFAULT_GOAL := default
help:
	@echo please use:
	@echo *all - to build all (user-space examples & kernel module)
	@echo *program - to build user-space examples
	@echo *debug - to build user-space examples (WITH DEGUG enabled)
	@echo *kernel - to build kernel module
	@echo *run - start one of user-space examples	

program:
	make -C EXAMPLES build
debug:
	make -C EXAMPLES debug
run:
	make -C EXAMPLES run

kernel:
	make -C KERNEL_MODULE build

all:	program	kernel

clean:
	make -C EXAMPLES clean
	make -C KERNEL_MODULE clean

default:	program	run