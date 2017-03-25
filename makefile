CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0



all:
	$(CC) $(CFLAGS) EXAMPLES/getStatus.cpp -o example-getStatus $(LFLAGS)
	$(CC) $(CFLAGS) EXAMPLES/moveTo.cpp -o example-moveTo $(LFLAGS)
	$(CC) $(CFLAGS) EXAMPLES/getHeight.cpp -o example-getHeight $(LFLAGS)
clean:
	rm -f example-moveTo example-getStatus example-getHeight
run:
	sudo ./example-getStatus
check:
	@echo -n "libusb-1.0: "
	@ldconfig -p | grep -q libusb-1.0 && echo "OK" || echo "ERROR please install libusb-1.0"

	@echo -n "usb device: "
	@lsusb -v -d 12d3:0002 && echo "OK" || echo "ERROR"
