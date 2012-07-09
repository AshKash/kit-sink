#!/usr/bin/perl

# uartd - this script will create a "virtual terminal" interface to the
# simulated mote UART.

# the PC mote simulator connects the UART to 127.0.0.1:8765
# this script will listen on that port to accept connection from a mote.
# once connected to a mote, it will wait for a second connection on a
# second port (defined below) to which the user connects a client
# (uartc.pl)  the client will then be able to send and receive
# bytes over the simulated UART.

$uart_port = '8765';
$client_port = '7654';
$maxclients = 10;

use Socket;

$tcp = getprotobyname('tcp');

print "Mote UART simulation daemon\n";

$packed_ip = inet_aton("127.0.0.1");

socket(UartSock, PF_INET, SOCK_STREAM, $tcp) or die "UART socket: $!\n";
setsockopt(UartSock, SOL_SOCKET, SO_REUSEADDR, pack("l", 1)) or die "UART setsockopt: $!\n";
bind(UartSock, sockaddr_in($uart_port, INADDR_LOOPBACK)) or die "UART bind: $!\n";

socket(ClientSock, PF_INET, SOCK_STREAM, $tcp) or die "Client socket: $!\n";
setsockopt(ClientSock, SOL_SOCKET, SO_REUSEADDR, pack("l", 1)) or die "Client setsockopt: $!\n";

bind(ClientSock, sockaddr_in($client_port, INADDR_LOOPBACK)) or die "Client bind: $!\n";

$SIG{INT} = 'cleanup';

listen(ClientSock, $maxclients);
listen(UartSock, $maxclients);

# main loop - send SIGINT to quit server.
while (1) {
  # accept "client" connection first.
  print "Waiting for client...\n";
  $clientaddr = accept(Client, ClientSock);
  select Client;
  $| = 1;
  select STDOUT;
  print "Client connected.\n";
  print Client "Connected to uartd.\n";
  print("Waiting for UART connection...\n");
  print(Client "Waiting for UART connection...\n");
  # then accept simulated mote UART connection
  print "Waiting for UART...\n";
  $uartaddr = accept(Uart, UartSock);
  # once we have a pair, fork off the i/o relaying loop code
  unless ($child = fork) {
    die "can't fork!\n" unless defined $child;
    clientloop();
    exit;
  }
  close(Client);
  close(Uart);
  push @ckids, $child;
}

sub cleanup {
  print "SIGINT detected; killing children\n";
  kill 15, @ckids;
  print "exiting...\n";
  exit;
}

sub clientloop {
  # we are handling SIGINT from the main code.
  $SIG{INT} = 'IGNORE';
  print "UART connected!\n";
  print "forked ($$)\n";
  print Client "UART connected.\n";
  select Uart;
  $| = 1;
  select STDOUT;
  $cbits = '';
  $ubits = '';
  $ufile = fileno(Uart);
  $cfile = fileno(Client);
  vec($rbits, $ufile, 1) = 1;
  vec($rbits, $cfile, 1) = 1;
#  print "ufile = $ufile; cfile = $cfile\n";
  # this should be changed to exit if one or both sockets is closed.
  while(1) {
    $tmpbits = "";
    ($nfound, $tmleft) = select($tmpbits = $rbits, undef, undef, undef);
#    print "ufile: $ufile; cfile: $cfile; select: ";
#    for($i=0;$i<8;$i++){
#      print "1" if vec($tmpbits, $i, 1);
#      print "0" unless vec($tmpbits, $i, 1);
#    }
#    print "\n";
    if (vec($tmpbits, $cfile, 1)) {
      $data = "";
      $nread = sysread(Client, $data, 1);
      if ($nread) { $nwritten = syswrite(Uart, $data, $nread) }
      printf "$$ Client: %s %x\n", $data, ord $data;
    }
    if (vec($tmpbits, $ufile, 1)){
      $data = "";
      $nread = sysread(Uart, $data, 1);
      if ($nread) { $nwritten = syswrite(Client, $data, $nread) }
      printf "$$ Uart: %s %x\n", $data, ord $data;
    }
  }
}

#code for displaying binary stored in a vector
#for($i=0;$i<8;$i++){print "1" if vec($tmpbits,$i,1);print "0" unless vec($tmpbits,$i,1);}

