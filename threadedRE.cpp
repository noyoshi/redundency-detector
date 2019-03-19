#include <iostream>
#include <pthread.h>
#include <stdio.h>

#include "hash.h"
#include "packet.h"

using namespace std;

void help() {
    /* Prints out a help message if the -h flag, or incorrect flags are given */
}

void report(int hits, int processedData, int redundantData) {
    /* Generates report for the program */
    printf("%d bytes processed\n", processedData);
    printf("%d hits\n", hits);
    printf("%d redundency detected\n", processedData / redundantData);
}

void analyzeFile(FILE * inputFile) {
    /* Producer that loops through the input file and fills a queue of packets */
}

void analyzePacket(packet p) {
    /* Consumer that gets packets from the queue and analyzes them */
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

    /* Data (in bytes) */
    int processedData = 0;
    int redundantData = 0;

    return 0;
}
