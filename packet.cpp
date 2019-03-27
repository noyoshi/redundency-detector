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
unsigned long long djb2Hash(unsigned char *str){
    unsigned long long hash = 5381;
    int c; 

    for (int i = 0; i < DATA_SIZE; i++){
        c = (str[i] == '\0') ? 0 : str[i]; 
        hash = ((hash << 5) + hash) + (unsigned long long) c;
    }
    /* fprintf(stderr, "[DJ] %llu\n", hash); */
    return hash;
}

packet * parsePacket(FILE * fp) {
    /* Parses the file we are reading and creates a single packet */
    uint32_t     packetLength;
    packet * p = NULL;

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
        /* Might not be a bad idea to pay attention to this return value */
        // We want to skip the first 52 bytes of the packer per the instructions
        check(fseek(fp, 56, SEEK_CUR));
        int dataLength = packetLength - 52;

        p = (packet *) calloc(1, sizeof(packet));
        if (p == NULL) ERROR;
        check(fread(p->data, 1, dataLength, fp));
        p->size = packetLength; // TODO is this the correct number?
    } else {
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

    return p;
}
