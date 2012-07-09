<?php
    if (isset($_COOKIE['firstname']))
        {
        $user = $_COOKIE['firstname'];
        $color= $_COOKIE['fontcolor'];
        }
    else
        {
        $user =  $_POST['user'];
        $color = $_POST['color'];
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

    <h1>tedd's show cookie stuff</h1>
    <hr>
   
    <h2>Hello: <?php echo( $user ); ?> </h2>
    <h2>Your color: <?php echo( $color ); ?> </h2>
   
    <hr>
   
    <br/>
     <?php
    // Another way to debug/test is to view all cookies

    echo ("<br/>");
    echo ("<pre>");
    echo ("Cookie info:\n");
    print_r($_COOKIE);
    echo("</pre>");
               
    ?>
    <p>
        <a><input type="button" value="back" onclick="history.go(-1)"></a>
    </p>
   
</body>
</html>
