/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <vector>
#include <stdio.h>

typedef struct _packet {
    /* bool isValid; */ 
    unsigned char data[2400]; /* 2400 bytes is max packet size */
    int hash; /* Hash of the packet */
    /* int length; /1* Length of the packet *1/ */
} packet;

int hashData(unsigned char[2400]); 
bool checkHash(int, packet *[30000]); 
bool checkContent(); 

packet* parsePacket(FILE *);
