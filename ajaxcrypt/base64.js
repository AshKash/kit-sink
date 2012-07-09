// public method for encoding
function _base64Encode(input) {
	var output = "";
	var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
	var i = 0;
	var _keyStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

	input = input.replace(/\r\n/g,"\n");
	
	var utftext = "";

	for (var n = 0; n < input.length; n++) {

		var c = input.charCodeAt(n);

		if (c < 128) {
			utftext += String.fromCharCode(c);
		}
		else if((c > 127) && (c < 2048)) {
			utftext += String.fromCharCode((c >> 6) | 192);
			utftext += String.fromCharCode((c & 63) | 128);
		}
		else {
			utftext += String.fromCharCode((c >> 12) | 224);
			utftext += String.fromCharCode(((c >> 6) & 63) | 128);
			utftext += String.fromCharCode((c & 63) | 128);
		}
	}
	
	input = utftext;

	while (i < input.length) {

		chr1 = input.charCodeAt(i++);
		chr2 = input.charCodeAt(i++);
		chr3 = input.charCodeAt(i++);

		enc1 = chr1 >> 2;
		enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
		enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
		enc4 = chr3 & 63;

		if (isNaN(chr2)) {
			enc3 = enc4 = 64;
		} else if (isNaN(chr3)) {
			enc4 = 64;
		}

		output = output +
		_keyStr.charAt(enc1) + _keyStr.charAt(enc2) +
		_keyStr.charAt(enc3) + _keyStr.charAt(enc4);

	}

	return output;
}

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
