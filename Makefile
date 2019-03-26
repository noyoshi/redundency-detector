CFLAGS=-Wall -O3

all: threadedRE

threadedRE: packet.o threadedRE.cpp
	g++ murmur3/murmur3.c packet.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o

clean:
	rm threadedRE packet.o optimized

optimize: packet.cpp threadedRE.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o
	g++ $(CFLAGS) packet.o threadedRE.cpp -o optimized -lpthread

debug: packet.o threadedRE.cpp
	g++ -D DEBUG_ALL murmur3/murmur3.c packet.o threadedRE.cpp -o threadedRE -lpthread

test: threadedRE optimize
	./test.sh

