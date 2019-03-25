/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <iostream>
#include <vector>
#include <pcap.h>
#include <stdio.h>
#include <vector>

#include "packet.h"
#include "config.h"

using namespace std;

/* Hashes the given data and returns the hashed data */
/* djb2 hash */
unsigned long long hashData(unsigned char *str){
    unsigned long long hash = 5381;
    int c; 

    for (int i = 0; i < DATA_SIZE; i++){
        c = (str[i] == '\0') ? 0 : str[i]; 
        hash = ((hash << 5) + hash) + c;
    }
    /* printf("current hash is %llu\n", hash); */
    return hash;
}

bool checkContent(packet* p1, packet* p2, int level) {
    /* Checks the actual content of the packet with the stored packet to confirm a match.
     * You will want to use memcmp, not strcmp as everything in the files is binary content.
     */
    // NOTE this is only returning true once... not sure if that is correct?
    // TODO chck accuracy

    // Stage 1: Compare Whole Data Arrays 
    if (level == 1){
        if (memcmp(&p1->data, &p2->data, DATA_SIZE) == 0){
            return true;
        } else {
            return false;
        }
    }
    // Stage 2: Jump Through Arrays in Chunks
    else if (level == 2){
        // for (int i = 0; i < 2400; ++i){

        // }
    }

    return false;
}

void freePackets() {
    /* Frees the packets in the packet holder */
    // TODO free the packets!
}

packet* parsePacket(FILE * fp) {
    /* Parses the file we are reading and creates a single packet */
    uint32_t     packetLength;

    packet *p = NULL;

    /* Skip the ts_sec field */
    /* check(fseek(fp, 4, SEEK_CUR)); */

    /* Skip the ts_usec field */
    /* check(fseek(fp, 4, SEEK_CUR)); */

    // Instead of the above?
    check(fseek(fp, 8, SEEK_CUR));

    /* Read in the incl_len field */
    check(fread(&packetLength, 4, 1, fp));

    /* Skip the orig_len field */
    // If you add this back, make sure to subtract 4 from bottom fseeks
    /* check(fseek(fp, 4, SEEK_CUR)); */

    /* Let’s do a sanity check before reading */
    if(packetLength < DATA_SIZE && packetLength > 128) {
        /* printf("Packet length was %d\n", packetLength); */
        /* Might not be a bad idea to pay attention to this return value */
        // We want to skip the first 52 bytes of the packer per the instructions
        check(fseek(fp, 56, SEEK_CUR));
        int dataLength = packetLength - 52;
        // packetData contains the byte data we care about
        // TODO intialize a new packet data structure here
        // TODO save the packet to the shared data structure here
        // TODO set something so that the consumer can know that there is
        // some new data to work on
        p = (packet*) calloc(1, sizeof(packet));
        if (p == NULL) ERROR;
        check(fread(p->data, 1, dataLength, fp));
        p->hash =  hashData(p->data) % HASHTABLE_SIZE;
        p->size = packetLength; // TODO is this the correct number?
    } else {
        /* printf("Skipping %d bytes ahead - packet is wrong size\n", packetLength); */

        // If we fseek past the end of a file and never read, "eof" won't be set
        // Fix -> do a garbage fread that will set the "eof" if we are past the
        // end of the file. Otherwise, if the last packet in the file is
        // something we want to skip, then we will never read and only seek -
        // this not setting "eof" and never ending the reading loop for the
        // file.
        uint32_t garbage;
        check(fseek(fp, packetLength, SEEK_CUR));
        check(fread(&garbage, 4, 1, fp));
    }

    /* TODO this might not work because if this is ran in a thread...? Weird
     * stuff happens? But since we are mallocing it idk maybe its ok?
     */
    return p;
}
