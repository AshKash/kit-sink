//Decodes Base64 formated data
function _base64Decode($data) {
    // strip none base64 characters
    $data = $data.replace(/[^a-z0-9\+\/=]/ig, '');
    if (typeof(atob) == 'function') {
        //use internal base64 functions if available (gecko only)
        return atob($data);
    }
    var $b64map = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
    var $byte1, $byte2, $byte3;
    var ch1, ch2, ch3, ch4;
    //array is used instead of string because in most of browsers working with large arrays is faster than working with large strings
    var result = new Array();
    var j=0;
    while (($data.length%4) != 0) {
        $data += '=';
    };
	
    for (var i=0; i<$data.length; i+=4) {
        ch1 = $b64map.indexOf($data.charAt(i));
        ch2 = $b64map.indexOf($data.charAt(i+1));
        ch3 = $b64map.indexOf($data.charAt(i+2));
        ch4 = $b64map.indexOf($data.charAt(i+3));

        $byte1 = (ch1 << 2) | (ch2 >> 4);
        $byte2 = ((ch2 & 15) << 4) | (ch3 >> 2);
        $byte3 = ((ch3 & 3) << 6) | ch4;

        result[j++] = String.fromCharCode($byte1);
        if (ch3 != 64) {
            result[j++] = String.fromCharCode($byte2);
        }
        if (ch4 != 64) {
            result[j++] = String.fromCharCode($byte3);	
        }
    };

    return result.join('');
};

//Decodes Hex(base16) formated data
function _hexDecode($data){
    var $b16digits = '0123456789abcdef';
    var $b16map = new Array();
    for (var i=0; i<256; i++) {
        $b16map[$b16digits.charAt(i >> 4) + $b16digits.charAt(i & 15)] = String.fromCharCode(i);
    };
    if (!$data.match(/^[a-f0-9]*$/i)) {
        // return false if input $data is not a valid Hex string
        return false;
    }
	
    if ($data.length % 2) {
        $data = '0'+$data;
    }
		
    var result = new Array();
    var j=0;
    for (var i=0; i<$data.length; i+=2) {
        result[j++] = $b16map[$data.substr(i,2)];
    };

    return result.join('');
};

function _rc4Encrypt($key, $pt) {
    s = new Array();
    for (var i=0; i<256; i++) {
        s[i] = i;
    };
    var j = 0;
    var x;
    for (i=0; i<256; i++) {
        j = (j + s[i] + $key.charCodeAt(i % $key.length)) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
    };
    i = 0;
    j = 0;
    var ct = '';
    for (var y=0; y<$pt.length; y++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        x = s[i];
        s[i] = s[j];
        s[j] = x;
        ct += String.fromCharCode($pt.charCodeAt(y) ^ s[(s[i] + s[j]) % 256]);
    };
    return ct;
};

function _rc4Decrypt($key, $ct) {
    return _rc4Encrypt($key, $ct);
};

/* 
 * Ashwin - Begin kab code
 */
function _createCookie($name,$value,$days) {
    if ($days) {
        var $date = new Date();
        $date.setTime($date.getTime()+($days*24*60*60*1000));
        var $expires = "; expires="+$date.toGMTString();
    } else {
        var $expires = "";
    }
    document.cookie = $name+"="+$value+$expires+"; path=/";
};

function _eraseCookie($name) {
    _createCookie($name,"",-1);
};

function _readCookie($name) {
    var $nameEQ = $name + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') {
            c = c.substring(1,c.length);
        };
        if (c.indexOf($nameEQ) == 0) {
            return c.substring($nameEQ.length, c.length);
        }
    };
    return null;
};

function kab_onLoad() {
    //ck=_readCookie("kab_secret");
    ck = 'DEADFACE123123123123123400000000';
    if (ck == null) {
        alert("You tried to use adblock? hahaha");
        return;
    }
    //_eraseCookie("kab_secret");  // cookie expiry is needed 
    alert("Press OK to start decoding page");
    var kabs = document.getElementsByName("kab");
    for (i=0; i < kabs.length; i++) {
        var enc;
        enc = kabs[i].innerHTML;
        //alert(enc);
        enc = _base64Decode(enc);
        var plaintext = _rc4Decrypt(_hexDecode(ck.toLowerCase()), enc);
        //alert(plaintext);
        kabs[i].innerHTML = plaintext;
    };
    alert("decryptok, #" + kabs.length);
};
