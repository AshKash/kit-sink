#
# This script provied means to disable AdBlock software installed on a browser.
#
# It encodes a given html (file:/// or http://) in such a way that it can
# decode itself given the correct password.
# You need to enable javascript to make this work.
# Also, any content deemed as an Ad is not encrypted and can be rendered.
# One way to send the secret to decode is via a cookie when the Ad is pulled.
#
# Copyright Ashwin Kashyap <ashwin.kashyap@thomson.net> All Rights reserved.
#

# TODO divs might overlap:
# <div name="kab">...<div id=ad> .... </div>... </div>
# In the above example, the second </div> is (incorrectly) assumed to close the kab_div.

import sys
import os
import re
from binascii import hexlify, unhexlify
from base64 import standard_b64encode as b64encode
from sgmllib import SGMLParser
import htmlentitydefs
import M2Crypto.EVP as EVP

# This file is frequently updated and is freely available from
# http://easylist.adblockplus.org/adblock_rick752.txt
FILTER_FILE = "adblock_rick752.txt" 

# These vars control how you would like to push the decode routines
EMBED_KAB = True
XXXKAB = r"""eval(function(p,a,c,k,e,d){e=function(c){return(c<a?"":e(parseInt(c/a)))+((c=c%a)>35?String.fromCharCode(c+29):c.toString(36))};if(!''.replace(/^/,String)){while(c--){d[e(c)]=k[c]||e(c)}k=[function(e){return d[e]}];e=function(){return'\\w+'};c=1};while(c--){if(k[c]){p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c])}}return p}('b Q(d){d=d.1F(/[^a-1d-9\\+\\/=]/1J,\'\');g(1g(Y)==\'b\'){8 Y(d)}5 z=\'1A+/=\';5 K,G,F;5 M,D,o,B;5 h=w C();5 j=0;V((d.7%4)!=0){d+=\'=\'};f(5 i=0;i<d.7;i+=4){M=z.A(d.m(i));D=z.A(d.m(i+1));o=z.A(d.m(i+2));B=z.A(d.m(i+3));K=(M<<2)|(D>>4);G=((D&15)<<4)|(o>>2);F=((o&3)<<6)|B;h[j++]=r.t(K);g(o!=14){h[j++]=r.t(G)}g(B!=14){h[j++]=r.t(F)}};8 h.X(\'\')};b T(d){5 E=\'1l\';5 J=w C();f(5 i=0;i<l;i++){J[E.m(i>>4)+E.m(i&15)]=r.t(i)};g(!d.1t(/^[a-1v-9]*$/i)){8 1x}g(d.7%2){d=\'0\'+d}5 h=w C();5 j=0;f(5 i=0;i<d.7;i+=2){h[j++]=J[d.1y(i,2)]};8 h.X(\'\')};b 12(k,p){s=w C();f(5 i=0;i<l;i++){s[i]=i};5 j=0;5 x;f(i=0;i<l;i++){j=(j+s[i]+k.Z(i%k.7))%l;x=s[i];s[i]=s[j];s[j]=x};i=0;j=0;5 N=\'\';f(5 y=0;y<p.7;y++){i=(i+1)%l;j=(j+s[i])%l;x=s[i];s[i]=s[j];s[j]=x;N+=r.t(p.Z(y)^s[(s[i]+s[j])%l])};8 N};b 10(k,c){8 12(k,c)};b 1H(n,v,d){g(d){5 d=w 1j();d.1i(d.1h()+(d*17*R*R*1e));5 e="; 1a="+d.1c()}1f{5 e=""}L.U=n+"="+v+e+"; 1m=/"};b 1n(n){1p(n,"",-1)};b 1q(n){5 n=n+"=";5 H=L.U.1r(\';\');f(5 i=0;i<H.7;i++){5 c=H[i];V(c.m(0)==\' \'){c=c.W(1,c.7)};g(c.A(n)==0){8 c.W(n.7,c.7)}};8 13};b 1C(){I=\'1E\';g(I==13){O("1I 16 S 18 19? 1b");8}O("1k 1o S 1s 1u 1w");5 q=L.1B("1D");f(i=0;i<q.7;i++){5 u;u=q[i].P;u=Q(u);5 11=10(T(I.1z()),u);q[i].P=11};O("1G, #"+q.7)};',62,108,'|||||var||length|return|||function||||for|if|result||||256|charAt||ch3||kabs|String||fromCharCode|enc||new|||b64_map|indexOf|ch4|Array|ch2|b16_digits|b3|b2|ca|ck|b16_map|b1|document|ch1|ct|alert|innerHTML|_0|60|to|_2|cookie|while|substring|join|atob|charCodeAt|_1|plaintext|_3|null|64||tried|24|use|adblock|expires|hahaha|toGMTString|z0|1000|else|typeof|getTime|setTime|Date|Press|0123456789abcdef|path|_5|OK|createCookie|_6|split|start|match|decoding|f0|page|false|substr|toLowerCase|ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789|getElementsByName|kab_onLoad|kab|DEADFACE123123123123123400000000|replace|decryptok|_4|You|ig'.split('|'),0,{}))"""

class ABTest:
    """Detects if a url is an Ad or not.

    The patterns are imported from the adblock list"""
    def __init__(self, file=FILTER_FILE):
        fd = open(file, "r")
        pats=[]
        l=fd.readline()
        while l:
            l = l.strip()
            if not l:
                l=fd.readline()
                continue

            if l.startswith(("[", "!", "@@")):
                l=fd.readline()
                continue
            # Adblock uses the $ sign to encode context, we ca ignore it.
            l = l.partition("$")[0]
            l = self._escapex(l)
            l = l.replace("*", ".*")
            #print l

            pats.append(l)
            l=fd.readline()
        fd.close()

        pat="|".join(pats)
        self.matcher=re.compile(pat)

    def _escapex(self, s, metas="\.^$+?{[]|()"):
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

class Encrypter(SGMLParser):
    """Processes a html file and encrypts it using the given password

    Most of this is lifted from the BaseHTMLProcessor example in divintopython
    """
    def __init__(self, key):
        SGMLParser.__init__(self)
        self.key=key
        self.encreset()
        self.abtest = ABTest()

    def encreset(self):
        self.enc = EVP.Cipher(alg="rc4", salt="", key_as_bytes=0, key=self.key, iv="", op=1)

    def encrypt(self, text):
        # called from handle_data
        # Encrypt text block 

        if not text:
            return text
        ct=""
        ct += self.enc.update(text)
        ct += self.enc.final()
        return ct

    def reset(self):
        # extend (called from __init__ in ancestor)
        # Reset all data attributes
        self.pieces = []
        SGMLParser.reset(self)

    def unknown_starttag(self, tag, attrs):
        strattrs = "".join([' %s="%s"' % (key, value) for key, value in attrs])
        self.pieces.append("<%(tag)s%(strattrs)s>" % locals())

    def unknown_endtag(self, tag):         
        self.pieces.append("</%(tag)s>" % locals())

    def handle_charref(self, ref):         
        self.pieces.append("&#%(ref)s;" % locals())

    def handle_entityref(self, ref):       
        self.pieces.append("&%(ref)s" % locals())
        if htmlentitydefs.entitydefs.has_key(ref):
            self.pieces.append(";")

    def handle_data(self, text):           
        self.pieces.append(text)

    def handle_comment(self, text):        
        self.pieces.append("<!--%(text)s-->" % locals())

    def handle_pi(self, text):             
        self.pieces.append("<?%(text)s>" % locals())

    def handle_decl(self, text):
        self.pieces.append("<!%(text)s>" % locals())

    def isAttrAd(self, attrs):
        "return if any attr src refers to an ad"
        for tp in attrs:
            if tp[0].lower() == "src":
                if self.abtest.isAd(tp[1]):
                    print "Ad:", tp[1]
                    return True

        return False

    def encAttrs(self, attrs):
        "if any attr src is not an ad, it is encrypted"
        attrx = []  # contains encrypted urls
        for tp in attrs:
            if tp[0].lower() == "src":
                if self.abtest.isAd(tp[1]):
                    attrx += (tp,)
                    continue
                
            attrx += [(tp[0], self.encrypt(tp[1])),]
        return attrx

    def start_head(self, attrs):
        "Add kab.js script in this section"
        self.unknown_starttag("head", attrs)
        if EMBED_KAB:
            self.unknown_starttag("script", [("type", "text/javascript"),])
            self.pieces.append(XXXKAB)
        else:
            self.unknown_starttag("script", [("src", "xxxkab.js")])
        self.unknown_endtag("script")
        
    def start_body(self, attrs):
        #print "body start"
        attrx = []
        onload = False
        for tp in attrs:
            if tp[0].lower() == "onload":
                attrx += [(tp[0], "kab_onLoad();" + tp[1]),]
                onload = True
                break
        if not onload:
            attrx += [("onLoad", "kab_onLoad();"),]
        self.unknown_starttag("body", attrx)
        self.unknown_starttag("div", [("name", "kab"),])

    def end_body(self):
        self.unknown_endtag("div_kab")
        self.unknown_endtag("body")

    def start_xxx(self, xxx, attrs):
        "common method to handle some types of start tags"
        if self.isAttrAd(attrs):
            self.unknown_endtag("div_kab")
            self.unknown_starttag(xxx, attrs)
        else:
            # encrypt like normal content
            self.unknown_starttag(xxx, attrs)

    def end_xxx(self, xxx):
        "common method to handle some types of end tags"
        self.unknown_endtag(xxx)
        self.unknown_starttag("div", [("name", "kab"),])
        
    def start_img(self, attrs):
        if self.isAttrAd(attrs):
            self.unknown_endtag("div_kab")
            self.unknown_starttag("img", attrs)
            self.unknown_starttag("div", [("name", "kab"),])
        else:
            # pass it through (img url will be encrypted)
            self.unknown_starttag("img", attrs)
    
    def start_iframe(self, attrs):
        self.start_xxx("iframe", attrs)
        
    def end_iframe(self):
        self.end_xxx("iframe")

    def output(self, enc=1):
        "return processed html that is encrypted"

        if not enc:
            return "".join(self.pieces)
        
        encpieces = []
        pieces = []
        mode=0   # 0 is plain, 1 is enc
        for p in self.pieces:
            if p.startswith('<div name="kab">'):
                pieces.append(p)
                mode=1
                continue
            if p.startswith("</div_kab>"):
                mode=0
                p="</div>"

            if mode == 1:
                encpieces.append(p)
            else:
                enc = self.encrypt("".join(encpieces))
                enc = b64encode(enc)
                pieces.append(enc)
                pieces.append(p)
                self.encreset()
                encpieces = []
                
        return "".join(pieces)

def encrypt(url, key):
    """fetch URL and encrypt using key
    """    
    import urllib                      
    sock = urllib.urlopen(url)         
    htmlSource = sock.read()           
    sock.close()                       
    parser = Encrypter(key)
    parser.feed(htmlSource)
    parser.close()         
    return parser.output() 

def test(url, key):
    """test encryption against URL"""
    outfile = "enc.html"
    fsock = open(outfile, "wb")
    fsock.write(encrypt(url, key))
    fsock.close()
    import webbrowser
    webbrowser.open_new(outfile)

if __name__ == "__main__":
    #test("file:///home/kashyapa/kab/readme.html", unhexlify("DEADFACE123123123123123400000000"))
    test("file:///home/kashyapa/kab/test.html", unhexlify("DEADFACE123123123123123400000000"))
    #test("http://www.engadget.com/", unhexlify("DEADFACE123123123123123400000000"))
    #test("http://slashdot.org/", unhexlify("DEADFACE123123123123123400000000"))
    #test("http://reddit.com/", unhexlify("DEADFACE123123123123123400000000"))
    #test("http://finance.yahoo.com/", unhexlify("DEADFACE123123123123123400000000"))
