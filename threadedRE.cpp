#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unistd.h"

#include "packet.h"
#include "config.h"

#include "murmur3/murmur3.h"

/* Generates the report */
#define REPORT \
    { printf("%ld bytes processed\n%d hits\n%ld%% redundency detected\n", \
        totalBytesProcessed, hits, (totalRedundantBytes * 100) / totalBytesProcessed);} \

// Figure out how close we can get to 64 MB 
#define BLOOM_FILTER_SIZE 62500000
#define BUFFER_SIZE 15
#define N_HASHES 5
#define STRIDE 64

// Global mutex / condition variables and file pointer
typedef struct _thread__arg {
    FILE * fp;
    pthread_mutex_t * mutex;
    pthread_cond_t  * empty;
    pthread_cond_t  * fill;
} thread_args;

/* =================== Globals ===================== */

int sharedBufferIndex = 0;
int hits = 0;

long int totalBytesProcessed = 0;
long int totalRedundantBytes = 0;
bool combo = false;
int comboBytes = 0;

int maxListSize = 0;

bool doneReading = false;

packet * sharedBuffer[BUFFER_SIZE] = { NULL };
char bloomFilter[BLOOM_FILTER_SIZE] = { 0 };

int level = 1;

/* Debug stuff */
#ifdef DEBUG_ALL
long int maxDataInMemory = 0;
long int dataInMemory = 0;
long int numPackets = 0;
#endif

/* ================================================== */

void freePacket(packet * p) {
#ifdef DEBUG_ALL
    dataInMemory -= sizeof(packet);
#endif
    free(p);
}

void addPacketToBuffer(packet * p) {
    sharedBuffer[sharedBufferIndex++] = p;
    totalBytesProcessed += p->size;
#ifdef DEBUG_ALL
    dataInMemory += sizeof(packet);
    numPackets += 1;
    if (dataInMemory > maxDataInMemory) 
        maxDataInMemory = dataInMemory;
#endif
}

bool hashAndAdd(unsigned char * data, int size) {
    /* Hashes data and adds it to the bloom filter? */
    unsigned long djb2 = djb2Hash(data, size) % BLOOM_FILTER_SIZE;
    unsigned char murmur[128];
    MurmurHash3_x64_128(data, size, 1230, murmur);
    long hash = 0; 
    // By default assume that it is redundant
    bool redundant = true;
    for (int i = 1; i < N_HASHES + 1; i ++) {
        // The more dank this is, the better this is gonna be!
        hash = (int) murmur[0] + ((int) murmur[1] * 33) + djb2 * i;
        hash = hash % BLOOM_FILTER_SIZE;
        // If the bloom filter comes up with ANY 0s, then we KNOW that this is 
        // NOT redundant
        if (bloomFilter[hash] == 0) {
            redundant = false; 
        }

        // After the above check, we set it to 1, thereby "adding" it to the
        // bloom filter
        bloomFilter[hash] = 1;
#ifdef DEBUG_ALL
        printf("[HASH] %ld\n", hash);
#endif
    }
#ifdef DEBUG_ALL
    printf("[MURMUR] %d\n", (int) murmur[0]);
#endif

    if (redundant) {
        if (!combo){
            comboBytes = size;
            combo = true;
        }else 
            comboBytes += 1;
    }

    return redundant;
    // return redundant;
}

void checkAndAddToBloomFilter(packet * p) {
    // Checks and adds the packet to the bloom filter
    // TODO make this a loop that checks on windows of a certain size
    // If it is level 1, then the window size is the entire packet
    // This is making the assumption that the data size we are hashing 
    // is always going to be of the window size?
    unsigned char data[STRIDE];
    int dataOffset = 0; // Where in the data array we are
    if (level == 2) {
        while ((dataOffset + STRIDE) < p->size) {
            memcpy(data, p->data + dataOffset, STRIDE);
            // If there wasn't a match, but we are currently processing a previous match,
            // flush the data out and reset it
            if(!hashAndAdd(data, STRIDE)) {
                if (combo) {
                    combo = false;
                    totalRedundantBytes += comboBytes;
                    comboBytes = 0;
                    hits ++;
                }
            }
            dataOffset ++;
        }
    } else if (level == 1) {
        hashAndAdd(p->data, p->size);
    }

    if (combo) {
        combo = false;
        totalRedundantBytes += comboBytes;
        comboBytes = 0;
        hits ++;
    } 
    // TODO update hits and totalRedundantData
}

void * consumerThread(void * arg) {
    /* Consumer thread */

    thread_args * args = (thread_args *) arg;
    while (!doneReading || sharedBufferIndex > 0) {
        pthread_mutex_lock(args->mutex);
        // Spin
        while (!doneReading && sharedBufferIndex == 0) {
            pthread_cond_wait(args->fill, args->mutex);
        }
        if (sharedBufferIndex > 0) {
            packet * p = sharedBuffer[--sharedBufferIndex];
            assert(p != NULL);
            checkAndAddToBloomFilter(p);
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
    while (feof(args->fp) == 0) {
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
void help(char *progname, int status) {
    printf("usage: %s [-l LEVEL] [-t THREADS] file ...\n", progname);
    printf("\t -l LEVEL: set level of redundancy detection (default = 1)\n");
    printf("\t -t THREADS: # of threads to run (default = 2)\n");
    printf("\tMax threads: 10\n");
    printf("\tLevel 1: Detect redundancy on a whole packet payload basis using a hash function across the packet payload.\n");
    printf("\tLevel 2: Detect redundancy on sub-packet windows (minimum of 64 bytes).\n");
    exit(status);
}


void analyzeFile(FILE * fp, int numThreads) {
    /* Producer that loops through the input file and fills a queue of packets */

    /* Since we might be analyzing multiple files, we want to re-initialize the
     * global variables to zero
     */
    hits = 0;
    sharedBufferIndex = 0;
    doneReading = false;
    comboBytes = 0;
    combo = false;
#ifdef DEBUG_ALL
    numPackets = 0;
#endif

    /* Condition variables and lock */
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

    /* Frees the argument struct and generates the report */
    REPORT;
    free(threadArgs);

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
    int numThreads = 2;
    int c;

    if(argc == 1) help(argv[0], 1);

    // process command line arguments
    while((c = getopt(argc, argv, "hl:t:")) != -1){
        switch(c){
            case 'h':
                help(argv[0], 0);
            case 'l':
                if (!isNumber(optarg)) help(argv[0], 1);
                level = atoi(optarg);
                if (level != 1 && level != 2){
                    fprintf(stderr, "[ERROR] Invalid level specified\n");
                    help(argv[0], 1);
                }
                break;
            case 't':
                if (!isNumber(optarg)) help(argv[0], 1);
                numThreads = atoi(optarg);
                if (numThreads < 0 || numThreads > 9) {
                    fprintf(stderr, "[ERROR] Invalid number of threads\n");
                    help(argv[0], 1);
                }
                break;
            default:
                help(argv[0], 1);
                break;
        }
    }

    // process files remaining in command line arguments
    for (size_t i = optind; i < (size_t) argc; i++){
        FILE * inputFile = fopen(argv[i], "r");
        if (inputFile == NULL) ERROR;
        // Get the packet data from the file
        analyzeFile(inputFile, numThreads);

        /* Cleanup */
        fclose(inputFile);
    }

    return EXIT_SUCCESS;
}
