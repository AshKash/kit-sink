#!/usr/bin/env python

# process a adblock subscription list and complie a regex out of it

import re
import os, sys

FILTER_FILE="adblock_rick752.txt"

class ABTest:
    def __init__(self, file=FILTER_FILE):
        fd = open(file, 'r')
        pats=[]
        l=fd.readline()
        while l:
            l = l.strip()
            if not l:
                l=fd.readline()
                continue

            if l.startswith(('[', '!', '@@')):
                l=fd.readline()
                continue
            l = l.partition('$')[0]
            l = self._escapex(l)
            l = l.replace('*', '.*')
            #print l

            pats.append(l)
            l=fd.readline()
        fd.close()

        pat='|'.join(pats)
        self.matcher=re.compile(pat)

    def _escapex(self, s, metas='\.^$+?{[]|()'):
        "escape regex meta chars in string"
        for e in metas:
            s = s.replace(e, "\\" + e)

        return s

    def isAd(self, url):
        #m = matcher.match(url)
        m = self.matcher.search(url)
        if m != None:
            return True
        return False

#Test program
if __name__ == '__main__':
    o = ABTest()
    print o.isAd(sys.argv[1])
