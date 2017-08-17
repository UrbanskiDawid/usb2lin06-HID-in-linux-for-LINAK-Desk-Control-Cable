.DEFAULT_GOAL := controler

CC=g++
CFLAGS=-std=c++1y
IFLAGS=-I../../ -I/usr/include/libusb-1.0
FLAGS=$(CFLAGS) $(IFLAGS)

clean:
	rm -f *.o

controler:
	$(CC) $(FLAGS) -o statusReport.o       -c statusReport.cpp
	$(CC) $(FLAGS) -o usb2linException.o   -c usb2linException.cpp
	$(CC) $(FLAGS) -o usb2lin06Controler.o -c usb2lin06Controler.cpp

debug: CFLAGS += -DDEBUG -g
debug: controler

