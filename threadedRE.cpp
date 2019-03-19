#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "hash.h"
#include "packet.h"
#include "debug.h"

using namespace std;

void help() {
    // TODO
    /* Prints out a help message if the -h flag, or incorrect flags are given */
}

void report(int hits, int processedData, int redundantData) {
    /* Generates report for the program */
    printf("%d bytes processed\n", processedData);
    printf("%d hits\n", hits);
    printf("%d redundency detected\n", processedData / redundantData);
}

void freePackets(packet * packetHolder[30000]) {
    for(int i = 0; i < 30000; i ++) {
        if (packetHolder[i] != NULL) {
            printf("%x\n", packetHolder[i]);
            /* free(packetHolder[i]); */
        }
    }
}

void analyzeFile(FILE * fp) {
    /* Producer that loops through the input file and fills a queue of packets */

    // Ignores the first 28 bytes of the file - which are global header
    // information
    check(fseek(fp, 28, SEEK_SET));
    uint32_t     packetLength;
    char         packetData[2400];
    int packetIndex = 0;

    // TODO make this a better data structure
    // this is just a temporary thing!
    packet * packetHolder[30000];

    /* Reads the input file */
    while(!feof(fp)) {
        // Parses out the packets from the file pointer
        packetHolder[packetIndex++] = parsePacket(fp);
    }

    /* Frees the packets */
    freePackets(packetHolder);
}

void analyzePacket(packet p) {
    /* Consumer that gets packets from the queue and analyzes them */
    // TODO create the hash for the packet, and save it on the packet
    // TODO check to see if the hash is in the hash data structure
    // TODO if the hash is there, check to see if the data is identical
    // TODO handle the match / no match
}

int main(int argc, char * argv[]) {
    /* Logic should be:
     * - Is the packet big enough? (>= 128 bytes)
     *  - If not, ignore
     * - Compute the hash for byte 52 -> end of the packet
     * - Does this hash exist in our data structure?
     * - If yes, then is it an exact match?
     */

    /* Constraints:
     * - Memory limit at 64 MB
     * - Needs to be multithreaded (use pthreads, condition variables)
     */

    /* Note: We should keep track of the # of hits, and the total amount of data
     * we processed, and the total amount of data we saved (total data size of
     * all redundant packets?)
     */
    int hits = 0;
    int RET_STATUS = EXIT_FAILURE;

    /* Data (in bytes) */
    int processedData = 0;
    int redundantData = 0;
    
    FILE * inputFile = fopen("data/Dataset-Small.pcap", "r");
    if (inputFile == NULL) {
        ERROR; 
        goto END;
    }
    analyzeFile(inputFile);
    fclose(inputFile);
    RET_STATUS = EXIT_SUCCESS;
END:
    return RET_STATUS;
}
