/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <iostream>
#include <vector>
#include <pcap.h>
#include <stdio.h>
#include <vector>

#include "packet.h"
#include "debug.h"

using namespace std;

/* Hashes the given data and returns the hashed data */
/* djb2 hash */
unsigned long hashData(unsigned char *str){
    unsigned long hash = 5381;
    unsigned char c;
    while ((c = *str++)){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        std::cout << "current hash is " << hash << std::endl;
    }
    return hash;
}

bool checkHash(unsigned long hash, vector<packet *> packetHolder) {
    
    for (auto u : packetHolder){
        if (hash == hashData(u->data)){
            return true;
        }
    }

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

void freePackets(vector<packet *> packetHolder) {
    /* Frees the packets in the packet holder */
    for (size_t i = 0; i < packetHolder.size(); i ++) {
        if (packetHolder[i] != NULL) free(packetHolder[i]);
    }
}

float getTotalData(vector<packet *> packetHolder) {
    float totalSize = 0;
    for (int i = 0; i < 30000; i ++) {
        if (packetHolder[i] != NULL) {
            totalSize += packetHolder[i]->size;
        }
    }
    return totalSize / 1000000;
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
        // We want to skip the first 52 bytes of the packer per the instructions
        check(fseek(fp, 52, SEEK_CUR));
        int dataLength = packetLength - 52;
        // packetData contains the byte data we care about 
        // TODO intialize a new packet data structure here
        // TODO save the packet to the shared data structure here
        // TODO set something so that the consumer can know that there is
        // some new data to work on
        p = (packet*) malloc(sizeof(packet));
        if (p == NULL) ERROR;
        check(fread(p->data, 1, dataLength, fp));
        p->hash = hashData(p->data);
        p->size = packetLength; // TODO is this the correct number?
    } else {
        printf("Skipping %d bytes ahead - packet is wrong size\n", packetLength);
        check(fseek(fp, packetLength, SEEK_CUR));
    }

    /* TODO this might not work because if this is ran in a thread...? Weird
     * stuff happens? But since we are mallocing it idk maybe its ok?
     */
    return p;
}
