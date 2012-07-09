#!/bin/sh

count=7
while [ $count -ne 0 ]
do
echo $count
./dv_hop $count > node.$count &
count=`expr $count - 1`
done

