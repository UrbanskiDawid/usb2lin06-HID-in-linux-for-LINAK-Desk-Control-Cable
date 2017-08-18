CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0
IFLAGS=-I ../ -I/usr/include/libusb-1.0 -I Controler

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
	$(CC) $(CFLAGS) ./Controler/*.o -o example-getStatus getStatus.cpp $(LFLAGS) $(IFLAGS)
	$(CC) $(CFLAGS) ./Controler/*.o -o example-getStatus getStatus.cpp $(LFLAGS) $(IFLAGS)
	$(CC) $(CFLAGS) ./Controler/*.o -o example-moveTo    moveTo.cpp $(LFLAGS) $(IFLAGS)
	$(CC) $(CFLAGS) ./Controler/*.o -o example-getHeight getHeight.cpp $(LFLAGS) $(IFLAGS)

debug:	CFLAGS += -DDEBUG -g
debug:	DEBUG=debug
debug:	build

clean:
	make -C Controler clean
	rm -f example-moveTo example-getStatus example-getHeight

run:
	sudo ./example-getStatus
