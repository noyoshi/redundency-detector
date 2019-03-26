#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <list>

#include "packet.h"
#include "config.h"
#include "unistd.h"
#include "murmur3/murmur3.h"


using namespace std;

/* This is the gloabl struct that we will want to modify in the consumer
 * threads, eg we should increment hits and totalRedundantBytes when the
 * consumer encounters a redundant packet
 */

/* Generates a report */
#define REPORT \
    { printf("%ld bytes processed\n%d hits\n%ld %% redundency detected\n", \
        totalBytesProcessed, hits, (totalRedundantBytes * 100) / totalBytesProcessed);} \

// 62 MB TODO see how close to 64 MB we can get
// Should also assume we have a buffer that is max full...
#define MEMORY_LIMIT 63900000
#define BLOOM_FILTER_SIZE 61000000
#define N_HASHES 5

// Global mutex / condition variables and file pointer
typedef struct _thread__arg {
    FILE * fp;
    pthread_mutex_t * mutex;
    pthread_cond_t  * empty;
    pthread_cond_t  * fill;
} thread_args;

// TODO change buffer size to be larger??? Figure out how this plays into the
// memory constraint of the assignment...
int BUFFER_SIZE = 15;
int sharedBufferIndex = 0;
int hits = 0;

long int totalBytesProcessed = 0;
long int totalRedundantBytes = 0;
long int packetsProcessed    = 0;
long int maxDataInMemory = 0;
long int dataInMemory = 0;
long int numPackets = 0;

int maxListSize = 0;

bool doneReading = false;

packet * sharedBuffer[15] = { NULL };
char bloomFilter[BLOOM_FILTER_SIZE] = { 0 };

packet * get() {
    // TODO consider taking this out as it is an extra subroutine call...
    sharedBufferIndex --;
    return sharedBuffer[sharedBufferIndex];
}

void freePacket(packet * p) {
    dataInMemory -= sizeof(packet);
    free(p);
}

void addPacketToBuffer(packet * p) {
    sharedBuffer[sharedBufferIndex++] = p;
    totalBytesProcessed += p->size;
    dataInMemory += sizeof(packet);
    numPackets += 1;
    if (dataInMemory > maxDataInMemory) 
        maxDataInMemory = dataInMemory;
}

void addPacketToHashTable(packet * p) {
    // TODO add the packet INFO to the bloom filter
    long djb2 = djb2Hash(p->data) % BLOOM_FILTER_SIZE;
    unsigned char murmur[128];
    MurmurHash3_x64_128(p->data, 2400, 1230, murmur);
    long hash = 0; 
    for (int i = 0; i < N_HASHES; i ++) {
        hash = (int) murmur[0] + djb2 * i;
        hash = hash % BLOOM_FILTER_SIZE;
        bloomFilter[hash] = 1;
        /* printf("%ld\n", hash); */
    }
}

int checkBloomFilter(packet * p) {
    // TODO check to see if the packet is in the bloom filter
    long djb2 = djb2Hash(p->data) % BLOOM_FILTER_SIZE;
    unsigned char murmur[128];
    MurmurHash3_x64_128(p->data, 2400, 1230, murmur);
    long hash = 0; 
    for (int i = 0; i < N_HASHES; i ++) {
        hash = (int) murmur[0] + djb2 * i;
        hash = hash % BLOOM_FILTER_SIZE;
        if (bloomFilter[hash] == 0) return 0; 
        /* printf("%ld\n", hash); */
    }
    return 1;
}

void * consumerThread(void * arg) {
    /* Consumer thread */
    // TODO implement the checking in here...
    // MAKE SURE TO FREE THE PACKET STRUCT!!
    /* Consumer that gets packets from the queue and analyzes them */
    // TODO check to see if the hash of the packet is in the hash data structure
    // TODO if the hash is there, check to see if the data is identical
    // TODO handle the match / no match

    thread_args * args = (thread_args *) arg;
    while (!doneReading || sharedBufferIndex > 0) {
        pthread_mutex_lock(args->mutex);
        // Spin
        while (!doneReading && sharedBufferIndex == 0) {
            pthread_cond_wait(args->fill, args->mutex);
        }
        if (sharedBufferIndex > 0) {
            packet * p = get();
            assert(p != NULL);
            if (checkBloomFilter(p) == 0) {
                addPacketToHashTable(p);
            } else {
                hits ++;
                totalRedundantBytes += p->size;
            }
            freePacket(p);
        }
        pthread_cond_signal(args->empty);
        pthread_mutex_unlock(args->mutex);
    }
    return 0;
}

void * producerThread(void * arg) {
    /* Producer thread - loops through the file and adds stuff to the buffer */
    thread_args * args = (thread_args *) arg;

    // Loop through the file and parse the packets
    while(feof(args->fp) == 0) {
        // Parses out the packets from the file pointer
        packet * p = parsePacket(args->fp);
        pthread_mutex_lock(args->mutex);
        // Wait while the buffer is full...
        while (sharedBufferIndex == BUFFER_SIZE)
            pthread_cond_wait(args->empty, args->mutex);

        // Pushes to the buffer
        if (p != NULL) 
            addPacketToBuffer(p);

        pthread_cond_signal(args->fill);
        pthread_mutex_unlock(args->mutex);
    }
    doneReading = true;
    pthread_cond_broadcast(args->fill);
    return 0;
}

/* Prints out a help message if the -h flag, or incorrect flags are given */
void help(char *progname) {
    printf("usage: %s [-l LEVEL] [-t THREADS] file ...\n", progname);
    printf("\t -l LEVEL: set level of redundancy detection (default = 1)\n");
    printf("\t -t THREADS: # of threads to run (default = 2)\n");
    printf("\tMax threads: 10\n");
    printf("\tLevel 1: Detect redundancy on a whole packet payload basis using a hash function across the packet payload.\n");
    printf("\tLevel 2: Detect redundancy on sub-packet windows (minimum of 64 bytes).\n");
    exit(0);
}


void analyzeFile(FILE * fp, int numThreads, bool output) {
    /* Producer that loops through the input file and fills a queue of packets */
    // TODO either make this into something that can be run in a thread, or call
    // a thread that reads the file

    /* Since we might be analyzing multiple files, we want to re-initialize the
     * global variables to zero
     */
    hits = 0;
    sharedBufferIndex = 0;
    doneReading = false;
    numPackets = 0;

    /* Condition variables and lock */
    // TODO idk if this breaks for multiple files...
    pthread_cond_t empty  = PTHREAD_COND_INITIALIZER;
    pthread_cond_t fill   = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Jump through the global header of the file */
    check(fseek(fp, 28, SEEK_SET));

    // Initializes a struct to hold arguments for the producer
    thread_args * threadArgs = (thread_args *) malloc(sizeof(thread_args));

    threadArgs->fp = fp;
    threadArgs->mutex = &mutex;
    threadArgs->empty = &empty;
    threadArgs->fill  = &fill;

    // Creates the producer thread
    pthread_t producer;

    if (pthread_create(&producer, NULL, producerThread, threadArgs) < 0) {
        // If the thread creation fails...
        // Make sure to free the args struct
        free(threadArgs);
        ERROR;
    }
    /* Reads the input file */
    /* We should stream stuff into the buffer and ONLY read in new packets when
     * we can - we should not make the buffer too big due to the memory
     * constraint...
     */
    pthread_t consumers[numThreads];

    // Make all the consumers
    for (size_t i = 0; i < (size_t) numThreads; i ++) {
        if (pthread_create(&consumers[i], NULL, consumerThread, threadArgs) < 0) {
            free(threadArgs);
            ERROR;
        }
    }

    // Wait for producer thread
    if (pthread_join(producer, NULL) < 0)
        ERROR;

    for (size_t i = 0; i < (size_t) numThreads; i ++) {
        if (pthread_join(consumers[i], NULL) < 0) ERROR;
    }

    /* Frees the argument struct */
    if (output)
        REPORT;
    free(threadArgs);
    /* printf("%ld redundant bytes\n", totalRedundantBytes); */

    /* For development */
#ifdef DEBUG_ALL
    float dataInMemory = (float) maxDataInMemory + sizeof(char) * BLOOM_FILTER_SIZE;
    fprintf(stderr, "%ld packets processed\n", numPackets);
    fprintf(stderr, "%.2f MB max used for storage\n", dataInMemory / 1000000.0f); 
#endif
}

bool isNumber(char * optarg) {
    /* Ensures that the optional argument (c-string) is a number) */
    char c;
    while ((c = *optarg++)) {
        if (!isdigit(c)) return false;
    }
    return true;
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
    /* Data (in bytes) */
    // NOTE due to the memory constraints on this program, we cannot save every
    // packet... keep track of this data as we go? might have to store this on
    // some variable that we pass to the threads instead...

    // TODO keep track of the size of the data structures we are using (more
    // like, how many packets are we putting on the heap?)
    // TODO: connect these parameters to the logic
    int level = 1;
    int numThreads = 2; // TODO: change to "optimal" when we know what that is
    int c;
    bool output = false;

    if(argc == 1) help(argv[0]);

    // process command line arguments
    while((c = getopt(argc, argv, "hl:t:o")) != -1){
        switch(c){
            case 'h':
                help(argv[0]);
            case 'l':
                if (!isNumber(optarg)) help(argv[0]);
                level = atoi(optarg);
                if(level != 1 && level != 2){
                    fprintf(stderr, "Error: invalid level specified");
                    exit(1);
                }
                break;
            case 't':
                if (!isNumber(optarg)) help(argv[0]);
                numThreads = atoi(optarg);
                break;
            case 'o':
                output = true;
                break;
            default:
                help(argv[0]);
                break;
        }
    }

    // process files remaining in command line arguments
    for(size_t i = optind; i < (size_t) argc; i++){
        FILE * inputFile = fopen(argv[i], "r");
        if (inputFile == NULL) ERROR;
        // Get the packet data from the file
        analyzeFile(inputFile, numThreads, output);

        /* Cleanup */
        fclose(inputFile);
    }

    return EXIT_SUCCESS;
}
