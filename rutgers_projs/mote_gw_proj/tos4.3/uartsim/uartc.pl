#!/usr/bin/perl

# uartc - this script is the client for the "virtual terminal" for the
# simulated mote UART.

# this script is the client interface to the uartd server.  connect this
# client, then run the mote simulation; the UART will then be connected to
# this client which uses STDIN and STDOUT for its local i/o

$client_port = $ARGV[0];

# default to 7654 if no port specified
unless ($client_port) { $client_port = "7654" }

use Socket;

$tcp = getprotobyname('tcp');

print "Mote UART simulation client\n";

socket(Client, PF_INET, SOCK_STREAM, $tcp) or die "Client socket: $!\n";
connect(Client, sockaddr_in($client_port, INADDR_LOOPBACK)) or die "Client connect: $!\n";

print "Connected to server.\n";

open(TTY, "+</dev/tty");
system "stty cbreak < /dev/tty > /dev/tty 2>&1";

select Client;
$| = 1;
select TTY;
$| = 1;
select STDOUT;
$rbits = "";
$sfile = fileno(TTY);
$cfile = fileno(Client);
vec($rbits, $sfile, 1) = 1;
vec($rbits, $cfile, 1) = 1;
until(eof Client) {
  $nfound = select($tmpbits = $rbits, undef, undef, undef);
  if (vec($tmpbits, $sfile, 1)) {
    $nread = sysread(TTY, $data, 1);
    unless (ord($data) == 6) {
      $nwritten = syswrite(Client, $data, $nread);
    }
    if (ord($data) == 6) {
      print "\nFile to send:\n";
      chomp($file = <STDIN>);
      print "sending $file...\n";
      sendfile($file);
    }
  }
  if (vec($tmpbits, $cfile, 1)) {
    $nread = sysread(Client, $data, 1);
    $nwritten = syswrite(TTY, $data, $nread);
  }
}

print("Client connection closed.\n");
exit;

sub sendfile {
  my $file;
  my $rbits;
  my $sfile;
  my $cfile;
  ($file) = @_;
  open(File, $file) or print "Can't open $file:  $!\n";
  select File;
  $| = 1;
  select STDOUT;
  $rbits = "";
  $sfile = fileno(File);
  $cfile = fileno(Client);
  vec($rbits, $sfile, 1) = 1;
  vec($rbits, $cfile, 1) = 1;  
  until(eof(File)) {
    $nfound = select($tmpbits = $rbits, undef, undef, undef);
    if (vec($tmpbits, $sfile, 1)) {
      $nread = read(File, $data, 1);
      $nwritten = syswrite(Client, $data, $nread);
      $nwritten = syswrite(TTY, $data, $nread);
    }
    if (vec($tmpbits, $cfile, 1)) {
      $nread = sysread(Client, $data, 1);
      $nwritten = syswrite(TTY, $data, $nread);
    }
  }
  close(File);
}

