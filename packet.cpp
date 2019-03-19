/* The implementations for decoding the packets from the file and saving them to
 * a data structure */
#include <iostream>
#include <vector>
#include <pcap.h>
#include <stdio.h>

#include "packet.h"

using namespace std;

vector<int> getPacketSizes() {
    /* Returns the packet sizes of all packets in the input file */
    vector<int> packetSizes;

    return packetSizes;
}


/* Example! Remove once we implement the file reader producer */
void DumpAllPacketLengths (FILE * fp) {
    // Ignores the first 28 bytes of the file - which are global header
    // information
    fseek(fp, 28, SEEK_SET);
    uint32_t     nPacketLength;
    char         theData[2400];

    /* Reads the input file */
    while(!feof(fp)) {
        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);

        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);

        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);

        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);

        /* Let’s do a sanity check before reading */
        if(nPacketLength < 2400 && nPacketLength > 64) {
            printf("Packet length was %d\n", nPacketLength);
            /* Might not be a bad idea to pay attention to this return value */
            fread(theData, 1, nPacketLength, fp);
        } else {
            printf("Skipping %d bytes ahead - packet is wrong size\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
            /* At this point, we have read the packet and are onto the next one */
    }
}

void processPacketFile() {
}

