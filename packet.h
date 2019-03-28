/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include "config.h"

typedef struct _packet {
    unsigned char   data[DATA_SIZE]; /* 2400 bytes is max packet size */
    uint32_t        size; /* Size of the packet */
} packet;

unsigned long long djb2Hash(unsigned char*, int); 
packet * parsePacket(FILE *);
