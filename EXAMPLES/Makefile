CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0


default:
	@echo this is Main Make file
	@echo please use:
	@echo *build - to build all EXAMPLES
	@echo *clean - cleanup after build
	@echo *run - start example-getStatus
	@echo *check - to see if all requirements
check:
	@echo -n "libusb-1.0: "
	@ldconfig -p | grep -q libusb-1.0 && echo "OK" || (echo "ERROR please install libusb-1.0\n$ sudo apt-get install libusb-1.0-0-dev"; exit 1)

	@echo -n "usb device: "
	@lsusb -v -d 12d3:0002 && echo "OK" || echo "ERROR";
build:
	$(CC) $(CFLAGS) getStatus.cpp -o example-getStatus $(LFLAGS)
	$(CC) $(CFLAGS) moveTo.cpp -o example-moveTo $(LFLAGS)
	$(CC) $(CFLAGS) getHeight.cpp -o example-getHeight $(LFLAGS)
clean:
	rm -f example-moveTo example-getStatus example-getHeight
run:
	sudo ./example-getStatus

