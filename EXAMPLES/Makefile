CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0
IFLAGS=-I ./Controler/ -I ../ -I/usr/include/libusb-1.0
FLAGS=$(CFLAGS) $(LFLAGS) $(IFLAGS)

DEBUG=
SRC_DIR=..

default:
	@echo this is Main Make file
	@echo please use:
	@echo *build - to build all EXAMPLES
	@echo *debug - to build all EXAMPLES with DEBUG enabled
	@echo *clean - cleanup after build
	@echo *run - start example-getStatus
	@echo *check - to see if all requirements

check:
	@echo -n "libusb-1.0: "
	@ldconfig -p | grep -q libusb-1.0 && echo "OK" || (echo "ERROR please install libusb-1.0\n$ sudo apt-get install libusb-1.0-0-dev"; exit 1)

	@echo -n "usb device: "
	@lsusb -v -d 12d3:0002 && echo "OK" || echo "ERROR";

build:
	make -C Controler $(DEBUG)
	$(CC) $(FLAGS) -o example-getStatus -c getStatus.cpp
	$(CC) $(FLAGS) -o example-moveTo    -c moveTo.cpp
	$(CC) $(FLAGS) -o example-getHeight -c getHeight.cpp

debug:	CFLAGS += -DDEBUG -g
debug:	DEBUG=debug
debug:	build

clean:
	make -C Controler clean
	rm -f example-moveTo example-getStatus example-getHeight

run:
	sudo ./example-getStatus
