# Redundency Detector

> One way to realize improved network efficiency is to identify cases where redundant data is being sent over the 
> particular network link. The idea is that an inefficient app will send the same data multiple times when the app 
> could have simply cached or saved the content to avoid sending it those multiple times. The challenge that one runs 
> into is that while it is relatively easy to capture large volumes of wireless network traffic, it can be relatively 
> tedious to capture how much redundancy might exist and more importantly
> how much of that redundancy might be removable (versus say simply running a compression algorithm like zip or bzip2).

### Goal
The goal is to write a fast, multi-threaded redundancy detector for pre-captured network traffic (via pcap files). 
While this project will be a bit prescriptive in terms of the higher level approach (condition variables, pthreads, 
memory limits), you may elect to use whatever data structures and strategy you deem appropriate to maximize performance.

### TODOS (specific)
- [x] Check accuracy for the data compare function
- [ ] Add a more sophisticated cache eviction policy
- [ ] Evaluate linear probing as a possible hash table implementation
- [ ] Benchmark each build
- [ ] Make sure that as few things as possible are being passed by value

Note: the buffer size should be proportional to the number of consumer threads
we will make, as the more consumer threads means that the less packets will be
waiting on the buffer, ie we can have a smaller buffer.  

Note: Linear probing appears to be slightly faster, but less accurate. Need to
see how much less accurate and if smarter caching could be used to improve.

https://people.cs.clemson.edu/~dhouse/courses/405/papers/optimize.pdf

### TODOs
- [x] Detect redundancy on a whole packet payload basis using a hash function across the packet payload.
- [ ] Detect redundancy on sub-packet windows (minimum of 64 bytes).

### Specifications 
Program invocation should take in several arguments, the level to run the program, the number of threads that can be used, and the various files to be processed in order by the system. You may presume that the number of files to process is limited to 10 or less. For the case where the level is not specified, you should run using Level 2. For the case where the number of threads is not specified, you should use your optimal value. You should mandate a minimum number of two threads to allow for at least one producer and one consumer thread (you may ignore the main thread).
