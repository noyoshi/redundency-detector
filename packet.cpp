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

bool checkContent(packet * p1, packet * p2, int level) {
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

