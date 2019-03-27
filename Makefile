CFLAGS=-Wall -O3

all: threadedRE

threadedRE: packet.o threadedRE.cpp murmur.o
	g++ murmur.o packet.o threadedRE.cpp -o threadedRE -lpthread

debug: packet.o threadedRE.cpp murmur.o
	g++ -D DEBUG_ALL murmur.o packet.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) -c -g packet.cpp -o packet.o

murmur.o: murmur3/murmur3.c
	gcc $(CFLAGS) -c -g murmur3/murmur3.c -o murmur.o

test: threadedRE
	scripts/test.sh

clean:
	rm threadedRE packet.o murmur.o

