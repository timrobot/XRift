#!/bin/sh

#just in case
ipcrm -M 9000 # this is the key

cd /home/ishan/xriftstuff/XRift
./recordmydesktop/src/recordmydesktop &
pid1=$!
sleep 1
./oculus/xrift &
pid2=$!
sleep 10000000
kill -9 $pid2
sleep 1
kill -9 $pid1
rm out*ogv
