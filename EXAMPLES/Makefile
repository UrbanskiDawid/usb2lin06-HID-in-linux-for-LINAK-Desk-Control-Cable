CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0

SRC_DIR=..

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

statusReportEx: 
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) -c ../statusReportEx.cpp -o statusReportEx.o

usb2lin06Controler: statusReportEx
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) -c ../usb2lin06Controler.cpp -o usb2lin06Controler.o

build: usb2lin06Controler
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) getStatus.cpp -I$(SRC_DIR) usb2lin06Controler.o statusReportEx.o -o example-getStatus 
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) moveTo.cpp    -I$(SRC_DIR) usb2lin06Controler.o statusReportEx.o -o example-moveTo
	$(CC) $(DEBUG) $(CFLAGS) $(LFLAGS) getHeight.cpp -I$(SRC_DIR) usb2lin06Controler.o statusReportEx.o -o example-getHeight 

debug: DEBUG = -DDEBUG
debug: build
	
clean:
	rm -f example-moveTo example-getStatus example-getHeight usb2lin06Controler.o statusReportEx.o
run:
	sudo ./example-getStatus

