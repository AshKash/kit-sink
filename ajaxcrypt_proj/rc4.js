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
  $key = _hexDecode($key.toLowerCase());
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
    ct += String.fromCharCode(($pt.charCodeAt(y) & 0xff) ^ s[(s[i] + s[j]) % 256]);
  };
    return ct;
};
 
function _rc4Decrypt($key, $ct) {
  return _rc4Encrypt($key, $ct);
};
