CFLAGS=-Wall -O3

all: threadedRE

threadedRE: packet.o threadedRE.cpp
	g++ $(CFLAGS) packet.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o

clean:
	rm threadedRE packet.o optimized

optimize: packet.cpp threadedRE.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o
	g++ $(CFLAGS) packet.o threadedRE.cpp -o optimized -lpthread

test: threadedRE optimize
	./test.sh

