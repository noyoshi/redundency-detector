/* For analyzing the packet that we decoded */
int hash(); 

/* Takes a hash and checks to see if that is in our data structure */
bool checkHash(); 

/* Checks the actual content of the packet with the stored packet to confirm
 * a match. 
 * You will want to use memcmp, not strcmp as everything in the files is
 * binary content. 
 */
bool checkContent(); 
