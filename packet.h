/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <vector>
#include <stdio.h>

std::vector<int> getPacketSizes(); 

void processPacketFile();
void DumpAllPacketLengths (FILE *); 

typedef struct _packet {
    bool isValid; 
    unsigned char data[2400]; /* 2400 bytes is max packet size */
    int hash; /* Hash of the packet */
} packet;
