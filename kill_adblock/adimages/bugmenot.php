<?php 
setcookie("kab_secret", "DEADFACE123123123123123400000000", time()+1*3600, "/");
header('Content-Type: image/gif');
readfile("bugmenot.gif"); 
?>

