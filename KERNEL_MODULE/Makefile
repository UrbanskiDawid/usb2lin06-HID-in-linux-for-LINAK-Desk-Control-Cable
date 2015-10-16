MODULE_NAME=usb2lin06
obj-m += $(MODULE_NAME).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules -lusb
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
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
