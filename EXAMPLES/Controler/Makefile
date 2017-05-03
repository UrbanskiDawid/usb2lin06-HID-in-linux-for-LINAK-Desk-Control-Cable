.DEFAULT_GOAL := controler

CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0

clean:
	rm -f *.o

controler:
	$(CC) $(CFLAGS) $(LFLAGS) -I../../ -o statusReport.o       -c statusReport.cpp 
	$(CC) $(CFLAGS) $(LFLAGS) -I../../ -o usb2linException.o   -c usb2linException.cpp 
	$(CC) $(CFLAGS) $(LFLAGS) -I../../ -o usb2lin06Controler.o -c usb2lin06Controler.cpp

debug: CFLAGS += -DDEBUG -g
debug: controler

