.DEFAULT_GOAL := controler

CC=g++
CFLAGS=-std=c++1y
IFLAGS=-I/usr/include/libusb-1.0/ -I../../
FLAGS=$(CFLAGS) -lusb-1.0 $(IFLAGS)

clean:
	rm -f *.o

controler:
	$(CC) $(CFLAGS) -o statusReport.o       -c statusReport.cpp       $(IFLAGS)
	$(CC) $(CFLAGS) -o usb2linException.o   -c usb2linException.cpp   $(IFLAGS)
	$(CC) $(CFLAGS) -o usb2lin06Controler.o -c usb2lin06Controler.cpp $(IFLAGS)

debug: CFLAGS += -DDEBUG -g
debug: controler

