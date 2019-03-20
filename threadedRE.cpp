#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>

#include "packet.h"
#include "debug.h"
#include "unistd.h"

using namespace std;

/* This is the gloabl struct that we will want to modify in the consumer
 * threads, eg we should increment hits and totalRedundantBytes when the
 * consumer encounters a redundant packet
 */
typedef struct _data {
    int hits;
    int totalBytesProcessed;
    int totalRedundantBytes;
} PacketData;

void help() {
    // TODO
    /* Prints out a help message if the -h flag, or incorrect flags are given */
}

void report(int hits, int processedData, int redundantData) {
    /* Generates report for the program */
    printf("%d bytes processed\n", processedData);
    printf("%d hits\n", hits);
    printf("%d redundency detected\n", processedData / (redundantData * 1000000));
}

PacketData* analyzeFile(FILE * fp) {
    /* Producer that loops through the input file and fills a queue of packets */

    // Ignores the first 28 bytes of the file - which are global header
    // information
    PacketData * returnData = (PacketData *) malloc(sizeof(PacketData));
    if (returnData == NULL) {
        return NULL;
    }

    // Initialize struct before we start consuming the file
    returnData->hits = 0;
    returnData->totalBytesProcessed = 0;
    returnData->totalRedundantBytes = 0;

    /* Jump through the global header of the file */
    check(fseek(fp, 28, SEEK_SET));
    uint32_t     packetLength;
    char         packetData[2400];
    int packetIndex = 0;

    // TODO make this a better data structure
    // this is just a temporary thing!
    vector<packet *> packetHolder;

    /* Reads the input file */
    /* We should stream stuff into the buffer and ONLY read in new packets when
     * we can - we should not make the buffer too big due to the memory
     * constraint...
     */
    while(!feof(fp)) {
        // Parses out the packets from the file pointer
        packet * p = parsePacket(fp);
        // TODO save some info about the packet? ie how large it is? maybe
        // TODO we should ONLY SAVE THE PACKET if the consumer decides we
        // should... instead, here, we should simply add the packet to the
        // buffer
        packetHolder.push_back(p);

        // TODO do some kind of signal to the consumer threads to let them know
        // there is more data?
    }

    fprintf(stderr, "%f total bytes\n", getTotalData(packetHolder));
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
     * - Needs to use a producer thread(s) to read the file, and consumer
     *   thread(s) to process the redundancy detection
     *   - We DO NOT NEED MORE THAN ONE CONSUMER / PRODUCER - I think we should
     *   only have one and move up from there? (it seems like ONE of these needs
     *   to be multithreaded though, eg we can either have multiple consumer or
     *   producer threads, but do not necessarily need multiple for both?)
     * - We need AT LEAST 2 extra threads (one consumer, one producer)
     */

    /* Note: We should keep track of the # of hits, and the total amount of data
     * we processed, and the total amount of data we saved (total data size of
     * all redundant packets / total data processed)
     */
    int hits = 0;
    int RET_STATUS = EXIT_FAILURE;

    /* Data (in bytes) */
    // NOTE due to the memory constraints on this program, we cannot save every
    // packet... keep track of this data as we go? might have to store this on
    // some variable that we pass to the threads instead...
    int processedData = 0;
    int redundantData = 0;

    int level = 1;
    int numThreads = 2; // TODO: change to "optimal" when we know what that is

    int c;
    while((c = getopt(argc, argv, "l:t:")) != -1){
        switch(c){
            case 'l':
                level = atoi(optarg);
                break;
            case 't':
                numThreads = atoi(optarg);
                break;
        }
    }

    FILE * inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) ERROR;
    // Get the packet data from the file
    PacketData * packetData = analyzeFile(inputFile);

    // TODO do something with packet data eg make the report

    /* Cleanup */
    free(packetData);
    fclose(inputFile);

    RET_STATUS = EXIT_SUCCESS;
    return RET_STATUS;
}
