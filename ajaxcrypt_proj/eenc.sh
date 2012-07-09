#!/bin/sh

# encrypt give file using openssl

KEY=DEADFACE123123123123123400000000
IV=0
openssl enc -rc4 -e -a -d -K $KEY -iv $IV -in $1 -out $1.enc

