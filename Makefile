CFLAGS=-Wall -O3

all: threadedRE

threadedRE: packet.o threadedRE.cpp murmur.o
	g++ murmur.o packet.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o

murmur.o: murmur3/murmur3.c
	gcc $(CFLAGS) -c -g murmur3/murmur3.c -o murmur.o

optimize: packet.cpp threadedRE.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o
	g++ $(CFLAGS) packet.o threadedRE.cpp -o optimized -lpthread

debug: packet.o threadedRE.cpp murmur.o
	g++ -D DEBUG_ALL murmur.o packet.o threadedRE.cpp -o threadedRE -lpthread

test: threadedRE optimize
	./test.sh

clean:
	rm threadedRE packet.o optimized


