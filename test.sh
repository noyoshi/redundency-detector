#!/bin/bash

DEVICE_TYPE=`uname -a | awk '{print $1}'`
for i in {1..9};
do
  echo "Timing threads = "$i
  for j in {1..9}; 
  do
      { time ./threadedRE -t $i -o data/Dataset-Medium.pcap > /dev/null; } 2>&1 \
        | grep "real" | awk '{print $2}' 
  done
done
