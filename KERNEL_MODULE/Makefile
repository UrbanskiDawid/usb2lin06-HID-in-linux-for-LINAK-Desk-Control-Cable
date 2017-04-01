MODULE_NAME=usb2lin06
obj-m += $(MODULE_NAME).o

default:
	@echo this is Kernel Module Makefile
	@echo please use:
	@echo *build - to compile
	@echo *clean - to clean after build
	@echo *start - to insert module
	@echo *stop  - to remove module
	@echo *info  - to get check if module was inserted
	@echo *test  - to test mobule by sending 4000 as target height
check:
	@if [ ! -d /lib/modules/$(shell uname -r)/build ]; then echo "ERROR: linux headers are missing.\nPlease install: $ sudo apt-get install linux-headers"; exit 1; fi
build:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules -lusb
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
start:
	sudo insmod $(MODULE_NAME).ko
stop:
	sudo rmmod $(MODULE_NAME).ko
info:
	sudo lsmod | grep $(MODULE_NAME)
test:
	cat /dev/usb2lin06
	echo -n "4000" > /dev/usb2lin06
	dmesg | tail -n 1
	cat /dev/usb2lin06
