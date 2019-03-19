/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <iostream>
#include <vector>
#include <pcap.h>
#include <stdio.h>

#include "packet.h"
#include "debug.h"

using namespace std;

/* void freePacket(packet * p) { */
/*     if (p -> data != NULL) { */
/*         free(p -> data); */
/*     } */
/*     free(p); */
/* } */

/* packet* allocPacket() { */
/*     packet * p = (packet *) malloc(sizeof(packet)); */

/*     p->data = (unsigned char *) malloc(sizeof(char) * 2400); */
/*     if (p->data == NULL) { */
/*         free(p); */
/*         ERROR; */
/*         return NULL; */
/*     } */

/*     return p; */
/* } */


packet* parsePacket(FILE * fp) {
    /* Parses a single packet from the file we are reading */
    uint32_t     packetLength;

    packet *p = NULL;

    /* Skip the ts_sec field */
    check(fseek(fp, 4, SEEK_CUR));

    /* Skip the ts_usec field */
    check(fseek(fp, 4, SEEK_CUR));

    /* Read in the incl_len field */
    check(fread(&packetLength, 4, 1, fp));

    /* Skip the orig_len field */
    check(fseek(fp, 4, SEEK_CUR));

    /* Let’s do a sanity check before reading */
    if(packetLength < 2400 && packetLength > 128) {
        printf("Packet length was %d\n", packetLength);
        /* Might not be a bad idea to pay attention to this return value */
        // packetData contains the byte data we care about 
        // TODO intialize a new packet data structure here
        // TODO save the packet to the shared data structure here
        // TODO set something so that the consumer can know that there is
        // some new data to work on
        p = (packet*) malloc(sizeof(packet));
        check(fread(p->data, 1, packetLength, fp));
        /* p->length = packetLength; // TODO does this work...? */
    } else {
        printf("Skipping %d bytes ahead - packet is wrong size\n", packetLength);
        check(fseek(fp, packetLength, SEEK_CUR));
    }

    /* TODO this might not work because if this is ran in a thread...? Weird
     * stuff happens? But since we are mallocing it idk maybe its ok?
     */
    return p;
}
