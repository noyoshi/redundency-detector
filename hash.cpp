/* For analyzing the packet that we decoded */
#include "hash.h"

using namespace std;

int hash() {
    /* Hashes the given data and returns the hashed data */
    return -1;
}

bool checkHash() {
    /* Takes a hash and checks to see if that is in our data structure */
    bool RETURN_STATUS = false;

    return RETURN_STATUS;
}

bool checkContent() {
    /* Checks the actual content of the packet with the stored packet to confirm
     * a match. 
     * You will want to use memcmp, not strcmp as everything in the files is
     * binary content. 
     */
    return false;
}
