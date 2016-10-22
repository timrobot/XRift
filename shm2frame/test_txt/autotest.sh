#!/bin/sh

#just in case
ipcrm -M 9000 # this is the key

./source &
pid1=$!
sleep 1
./sink &
pid2=$!
sleep 5
kill -9 $pid2
kill -9 $pid1
