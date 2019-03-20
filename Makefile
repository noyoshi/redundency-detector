CFLAGS=-Wall -g -c

all: threadedRE

threadedRE: packet.o
	g++ packet.o threadedRE.cpp -o threadedRE -lpthread

packet.o: packet.cpp
	g++ $(CFLAGS) packet.cpp -o packet.o

clean:
	rm threadedRE packet.o
