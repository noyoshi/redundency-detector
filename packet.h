/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <vector>
#include <stdio.h>

typedef struct _packet {
    /* bool isValid; */ 
    unsigned char   data[2400]; /* 2400 bytes is max packet size */
    int             hash; /* Hash of the packet */
    uint32_t        size; /* Size of the packet */
} packet;

unsigned long hashData(unsigned char*); 
bool checkContent(packet *, packet *, int); 
float getTotalData(std::vector<packet *>);

void freePackets(); 
packet* parsePacket(FILE *);
