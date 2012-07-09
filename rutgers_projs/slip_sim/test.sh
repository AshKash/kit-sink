#!/bin/sh

# change this to point to some big binary file
file=tmp
while [ 1 ]
do
count=200
while [ $count -ne 0 ]
do
echo nc $count
nc localhost 8765 < $file &
count=`expr $count - 1`
done

sleep 100
killall nc
killall nc
killall nc
killall nc
killall nc
killall -9 nc
done
