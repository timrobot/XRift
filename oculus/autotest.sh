#!/bin/sh

#just in case
ipcrm -M 9000 # this is the key

../recordmydesktop/src/recordmydesktop &
pid1=$!
sleep 1
./xrift &
pid2=$!
sleep 12
kill -9 $pid2
sleep 1
kill -9 $pid1
rm out*ogv
