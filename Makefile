CFLAGS=-Wall -g -c

all: threadedRE

threadedRE: packet.o hash.o 
	g++ packet.o hash.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) packet.cpp -o packet.o

hash.o: hash.cpp
	g++ $(CFLAGS) hash.cpp -o hash.o

clean:
	rm threadedRE packet.o hash.o
