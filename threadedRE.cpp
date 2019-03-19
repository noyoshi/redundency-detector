#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

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
    packet * packetHolder[30000] = { NULL };

    /* Reads the input file */
    while(!feof(fp)) {
        // Parses out the packets from the file pointer
        packet * p = parsePacket(fp);
        // TODO save some info about the packet? ie how large it is? maybe
        packetHolder[packetIndex++] = p;
        // TODO do some kind of signal to the consumer threads to let them know
        // there is more data?
    }

    /* Frees the packets */
    freePackets(packetHolder);
}

// TODO make this threadable (single argument)
void analyzePacket(packet p, packet * packetHolder[30000]) {
    /* Consumer that gets packets from the queue and analyzes them */
    // TODO check to see if the hash of the packet is in the hash data structure
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

    /* For level 2, we want to compute the match over a small window of the data
     * in the packet. If there is a match, then we keep going into the data to
     * see how far of a match we have?
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
    // NOTE due to the memory constraints on this program, we cannot save every
    // packet... keep track of this data as we go? might have to store this on
    // some variable that we pass to the threads instead...
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
