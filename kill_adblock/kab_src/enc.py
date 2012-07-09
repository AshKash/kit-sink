#!/usr/bin/env python

import sys
import os
from binascii import hexlify, unhexlify
import M2Crypto.EVP as EVP
from base64 import encodestring

CHUNK=4096
# no iv for ecb
key = unhexlify(sys.argv[1])
#key = sys.argv[1]
#print 'key:', key, 'len:', len(key)
html = open(sys.argv[2], 'r')
enc = EVP.Cipher(alg='rc4', salt='', iv='', key_as_bytes=0, key=key, op=1)

ct=''
while 1:
    data = html.read(CHUNK)
    if not data:
        break
    ct += enc.update(data)
    
ct += enc.final()
html.close()

if __name__=='__main__':
    print encodestring(ct)
    #print hexlify(ct)
