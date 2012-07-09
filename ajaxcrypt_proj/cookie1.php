<?php

    $user =  $_POST['user'];
    $color = $_POST['color'];
    $self =  $_SERVER['PHP_SELF'];

    if( ( $user != null ) and ( $color != null ) )
        {
          setcookie( "firstname", $user , time() + 86400 );        // 24 hours
          setcookie( "fontcolor", $color, time() + 86400 );
          header( "Location:cookie2.php" );
          exit();
        }
?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
    <meta http-equiv="content-type" content="text/html;charset=ISO-8859-1">
    <title>Stuff by tedd</title>       
</head>

<body>

    <h1>tedd's cookie stuff</h1>

    <hr>

    <form action ="https://p2pms.prin.am.thmulti.com/ajaxcrypt/cookie1.php" method = "post">
   
    Please enter your first name:
    <input type = "text" name = "user"><br><br>
   
    Please choose your favorite font color:<br>
    <input type = "radio" name = "color" value = "Red">Red
    <input type = "radio" name = "color" value = "Green">Green
    <input type = "radio" name = "color" value = "Blue">Blue
    <br><br>
    <input type = "submit" value = "submit">
    </form>

    <br/>
    <hr>   

</body>
</html>
</html>
