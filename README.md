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


### TODOs
- [ ] Detect redundancy on a whole packet payload basis using a hash function across the packet payload.
- [ ] Detect redundancy on sub-packet windows (minimum of 64 bytes).
