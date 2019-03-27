#!/bin/bash
CID=`docker ps | tail -n1 | awk '{print $1}'`

docker cp Makefile $CID:/root
docker cp packet.h $CID:/root
docker cp packet.cpp $CID:/root
docker cp threadedRE.cpp $CID:/root
docker cp config.h $CID:/root
docker cp scripts/test.sh $CID:/root
docker cp data/Dataset-Medium.pcap $CID:/root

