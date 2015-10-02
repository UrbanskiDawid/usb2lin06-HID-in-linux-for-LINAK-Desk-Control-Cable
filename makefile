CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0
OUT=usb2lin06-example

all:
	$(CC) $(CFLAGS) main.cpp -o $(OUT) $(LFLAGS)
clean:
	rm -f $(OUT)
run:
	sudo ./$(OUT)

go: all run
