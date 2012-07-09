#!/bin/sh

# will arrange proper pipes
# do mknod pipe p if pipe does not exist

echo args: $@
./aps-demo.tcl 0<pipe | ./aps-query $@ 1>pipe

