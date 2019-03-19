/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <vector>
#include <stdio.h>

typedef struct _packet {
    /* bool isValid; */ 
    unsigned char data[2400]; /* 2400 bytes is max packet size */
    /* int hash; /1* Hash of the packet *1/ */
    /* int length; /1* Length of the packet *1/ */
} packet;

packet* parsePacket(FILE *);

void freePacket(packet *);
packet * allocPacket();
