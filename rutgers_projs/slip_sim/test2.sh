#!/bin/sh

# this test does not loop and connect to tty_server directly
# change this to point to some big binary file
file=tmp
port=7654
count=500
while [ $count -ne 0 ]
do
echo nc $count
nc localhost $port < $file &
count=`expr $count - 1`
#sleep 1
done

