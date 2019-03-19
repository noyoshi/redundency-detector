/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <iostream>
#include <vector>
#include <pcap.h>
#include <stdio.h>

#include "packet.h"
#include "debug.h"

using namespace std;

int hashData(unsigned char data[2400]) {
    /* Hashes the given data and returns the hashed data */
    // TODO implement this
    return -1;
}

bool checkHash(int hash, packet * packetHolder[30000]) {
    return false;
}

bool checkContent() {
    /* Checks the actual content of the packet with the stored packet to confirm
     * a match. 
     * You will want to use memcmp, not strcmp as everything in the files is
     * binary content. 
     */
    return false;
}

packet* parsePacket(FILE * fp) {
    /* Parses the file we are reading and creates a single packet */
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
        if (p == NULL) ERROR;
        check(fread(p->data, 1, packetLength, fp));
        p->hash = hashData(p->data);
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
