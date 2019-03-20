#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <unistd.h>

#include "packet.h"
#include "debug.h"

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

typedef struct _thread__arg {
    FILE * fp;
    pthread_mutex_t * mutex;
    pthread_cond_t * empty;
    pthread_cond_t * fill;
} thread_args;

int BUFFER_SIZE = 10;
vector<packet *> sharedBuffer;
int hits = 0;
int totalBytesProcessed = 0;
int totalRedundantBytes = 0;
int packetsReadIn = 0;
int packetsProcessed = 0;
bool doneReading = false;
int sharedBufferSize = 0;
/* int count = 0; */

packet * get() {
    packet * p = sharedBuffer.back();
    sharedBuffer.pop_back();
    return p;
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
    while (!doneReading || sharedBufferSize > 0) {
        printf("%d != %d ? %d size = %d\n", packetsReadIn, packetsProcessed, doneReading, sharedBufferSize);
        pthread_mutex_lock(args->mutex);
        /* if (doneReading && (sharedBufferSize == 0)) { */
        /*     pthread_mutex_unlock(args->mutex); */
        /*     return 0; */
        /* } */
        puts("in consumer");
        printf("%d != %d ? %d size = %d\n", packetsReadIn, packetsProcessed, doneReading, sharedBufferSize);
        while (!doneReading && sharedBufferSize == 0) {
            pthread_cond_wait(args->fill, args->mutex);
        }

        // TODO do something with the packet
        sharedBuffer.pop_back();
        sharedBufferSize --;
        pthread_cond_signal(args->empty);
        pthread_mutex_unlock(args->mutex);
    }
    puts("consumer is done!");
    return 0;
}

void * producerThread(void * arg) {
    /* Producer thread - loops through the file and adds stuff to the buffer */
    thread_args * args = (thread_args *) arg;

    // Loop through the file and parse the packets
    while(!feof(args->fp)) {
        // Parses out the packets from the file pointer
        packet * p = parsePacket(args->fp);
        pthread_mutex_lock(args->mutex);
        // Wait while the buffer is full...
        // THIS MIGHT NOT WORK! TODO CHECK
        while (sharedBufferSize == BUFFER_SIZE) {
            pthread_cond_wait(args->empty, args->mutex);
        }
        // Pushes to the buffer
        sharedBuffer.push_back(p);
        sharedBufferSize ++;
        /* puts("PUT PACKET TO BUFFER"); */
        pthread_cond_signal(args->fill);
        pthread_mutex_unlock(args->mutex);
    }
    doneReading = true;
    return 0;
}


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

void analyzeFile(FILE * fp) {
    /* Producer that loops through the input file and fills a queue of packets */
    // TODO either make this into something that can be run in a thread, or call
    // a thread that reads the file

    /* Since we might be analyzing multiple files, we want to re-initialize the
     * global variables to zero
     */
    hits = 0;
    totalRedundantBytes = 0;
    totalBytesProcessed = 0;
    sharedBuffer.clear();

    /* Condition variables and lock */
    // TODO idk if this breaks for multiple files...
    pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
    pthread_cond_t fill  = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Jump through the global header of the file */
    check(fseek(fp, 28, SEEK_SET));
    uint32_t     packetLength;
    char         packetData[2400];
    int packetIndex = 0;

    // TODO initialize the producer thread and start it...
    thread_args * producerArgs = (thread_args *) malloc(sizeof(thread_args));

    producerArgs->fp = fp;
    producerArgs->mutex = &mutex;
    producerArgs->empty = &empty;
    producerArgs->fill  = &fill;

    pthread_t producer;

    if (pthread_create(&producer, NULL, producerThread, producerArgs) < 0) {
        // If the thread creation fails...
        // Make sure to free the args struct
        free(producerArgs);
        ERROR;
    }
    /* Reads the input file */
    /* We should stream stuff into the buffer and ONLY read in new packets when
     * we can - we should not make the buffer too big due to the memory
     * constraint...
     */
    // TODO initialize the consumer thread(s) and call them...
    int numThreads = 2;
    pthread_t consumers[numThreads];

    // Make all the consumers
    for (int i = 0; i < numThreads; i ++) {
        if (pthread_create(&consumers[i], NULL, consumerThread, producerArgs) < 0) {
            free(producerArgs);
            ERROR;
        }
    }

    // Wait for producer thread
    if (pthread_join(producer, NULL) < 0) ERROR;

    for (int i = 0; i < numThreads; i ++) {
        if (pthread_join(consumers[i], NULL) < 0) ERROR;
        puts("joined a thread!");
        printf("thread %d of %d done\n", i, numThreads);
    }

    /* Frees the argument struct */
    free(producerArgs);
}

int main(int argc, char * argv[]) {
    /* Logic should be:
     * - Is the packet big enough? (>= 128 bytes)
     *  - If not, ignore
     * - Compute the hash for byte 52 -> end of the packet
     * - Does this hash exist in our data structure?
     * - If yes, then is it an exact match?
     */
    // TODO input argument parsing

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

    FILE * inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) ERROR;

    // Call stuff
    analyzeFile(inputFile);

    // TODO do something with packet data eg make the report

    /* Cleanup */
    fclose(inputFile);

    RET_STATUS = EXIT_SUCCESS;
    return RET_STATUS;
}
