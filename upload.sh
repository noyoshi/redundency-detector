#!/bin/bash
CID=`docker ps | tail -n1 | awk '{print $1}'`

docker cp $1 $CID:/root

