CC=g++
CFLAGS=-std=c++1y
LFLAGS=-lstdc++ -lusb-1.0


all:
	$(CC) $(CFLAGS) EXAMPLES/getStatus.cpp -o example-getStatus $(LFLAGS)
	$(CC) $(CFLAGS) EXAMPLES/moveTo.cpp -o example-moveTo $(LFLAGS)
clean:
	rm -f example-moveTo example-getStatus
run:
	sudo ./example-getStatus
