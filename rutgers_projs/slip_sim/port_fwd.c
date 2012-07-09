#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

/* the port on which we listen */
#define UART_PORT 8765

/* where do we send the data? */
#define CLIENT_PORT 7654
#define CLIENT_NAME "localhost"

#define BUF_SIZE (1024 * 1024)

#define MAX_CONN 256
#define FD_SETSIZE (MAX_CONN)
#include <sys/time.h>

/* globals */
char buf[BUF_SIZE];
fd_set sock_read, sock_write, sock_except;
struct {
  int uart[MAX_CONN];
  int client[MAX_CONN];
  int len;    /* the two arrays are always equal */
} sock_pair;
int sock_max, accept_sock;

struct sockaddr_in uart_addr, client_addr;

/* closes sockets nicely */
int close_socks(int uart, int client)
{
  /* simple things... */
  if (uart != -1) {
    close(uart);
  }

  if (client != -1) {
    close(client);
  }

  return 1;
}

/* adds given int pair into the sock_pair list. returns true/false */
int add_sockpair(int uart, int client)
{
  int i;

  /* scan for next pos */
  for (i=0; i<MAX_CONN; i++) {
    if (sock_pair.uart[i] == -1 &&
	sock_pair.client[i] == -1) {
      sock_pair.uart[i] = uart;
      sock_pair.client[i] = client;
      sock_pair.len++;
      return 1;
    }
  }
  return -1;
}

/* deletes the given index and does clean up */
void del_sockindex(int index)
{
  int *uart, *client;
  uart = &sock_pair.uart[index];
  client = &sock_pair.client[index];

  /* check if sock exists */
  if (*uart == -1 &&
      *client == -1)
    return;
  if (sock_pair.len <= 0) {
    fprintf(stderr, "sock_pair.len < 0!!\n");
    return;
  }

  /* remove from fd set */
  FD_CLR(*uart, &sock_read);
  FD_CLR(*uart, &sock_write);
  FD_CLR(*uart, &sock_except);
  FD_CLR(*client, &sock_read);
  FD_CLR(*client, &sock_write);
  FD_CLR(*client, &sock_except);

  /* close */
  printf("Close: %d -> %d\n", *uart, *client);
  close_socks(*uart, *client);
  *uart = *client = -1;
  sock_pair.len--;
}

/* opposite of add */
int del_sockpair(int uart, int client)
{
  int i;

  for (i=0; i<MAX_CONN && sock_pair.len != 0; i++) {
    if (sock_pair.uart[i] == uart && sock_pair.client[i] == client) {
      del_sockindex(i);
      return 1;
    }

  }
  return -1;
}

/* exit handler. closes all socks nicely before exiting */
void exit_nice(void)
{
  int i;
 
  printf ("Exiting...\n");

  shutdown(accept_sock, 2);
  close(accept_sock);
  for (i=0; i < MAX_CONN && sock_pair.len !=0; i++){
    del_sockindex(i);
  }
}

/* catch sig pipe. this is mainly to prevent the process from dying on SIGPIPE */
void sig_pipe(int sig)
{
  printf ("SIGPIPE caught and ignored\n");
}


/* catch signals */
void sig_catch(int sig)
{
  exit(0);
}

/* inits all globals */
void init_structs(void)
{
  struct hostent *hp;
  int i;

  /* init the fd sets */
  FD_ZERO(&sock_read);
  FD_ZERO(&sock_write);
  FD_ZERO(&sock_except);

  /* init the sock structs */
  memset((char *) &uart_addr, 0, sizeof(uart_addr));
  uart_addr.sin_family = AF_INET;
  uart_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  uart_addr.sin_port = htons(UART_PORT);
  
  /* client socket struct */
  hp = gethostbyname(CLIENT_NAME);
  if (!hp) {
    printf("Can't get host entry for host %s\n", CLIENT_NAME);
    /* herror("gethostbyname"); */
    exit(-1);
  }
  memcpy(&client_addr.sin_addr, hp->h_addr, sizeof(client_addr.sin_addr));
  client_addr.sin_port = ntohs(CLIENT_PORT);
  client_addr.sin_family = AF_INET;

  /* fill the socket pair table */
  sock_pair.len = 0;
  for (i=0; i<MAX_CONN; i++) {
    sock_pair.uart[i] = sock_pair.client[i] = -1;
  }
}

/* reads from one of the fds and writes data to the other. may block */
int read_write(int read_fd, int write_fd)
{
  int r, w;

  r = recv(read_fd, buf, BUF_SIZE, MSG_DONTWAIT);
  /* since sock is non blocking, this means some serious error */
  if (r < 0) {
    perror("read");
    return r;
  } else if (r == 0) {
    /* connection has terminated */
    return -1;
  }

  w = write(write_fd, buf, r);
  if (w != r) {
    perror ("write");
    return -1;
  }

  printf("[%d->%d] ", read_fd, write_fd);
  fflush(stdout);

  return 1;
}

/* this will return the index of the next available pair that can be read
 * it will also write the readable fd and the writable fds into the args */
int get_next_fd (int *read_fd, int *write_fd)
{
  int i;
  int *uart_fd = sock_pair.uart;
  int *client_fd = sock_pair.client;
  
  for (i = 0; i < MAX_CONN; i++) {
    if (uart_fd[i] == -1 || client_fd[i] == -1) continue;

    if (FD_ISSET(uart_fd[i], &sock_read)) {
      FD_CLR(uart_fd[i], &sock_read);
      *read_fd = uart_fd[i];
      *write_fd = client_fd[i];
      return i;
    } else if (FD_ISSET(client_fd[i], &sock_read)) {
      FD_CLR(client_fd[i], &sock_read);
      *read_fd = client_fd[i];
      *write_fd = uart_fd[i];
      return i;
    }
  }
  return -1;
}

/* this will accept a connection on the given socket and connect the socket
 * to the other end */
int accept_connect()
{
  int new_uart = -1;
  int client_sock = -1;
  int r;
  socklen_t  cli_len;

  cli_len = sizeof(uart_addr);
  new_uart = accept(accept_sock, (struct sockaddr*) &uart_addr,
		    &cli_len);
  if (new_uart < 0) {
    perror ("accept");
    goto ACCEPT_FAILS;
  }

  /* connect to other end */
  client_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sock < 0) {
    perror ("socket");
    goto ACCEPT_FAILS;
  }
  r = connect(client_sock, (struct sockaddr *)&client_addr,
	      sizeof(client_addr));
  if (r < 0) {
    perror ("connect");
    goto ACCEPT_FAILS;
  }

  if (new_uart > client_sock && new_uart > sock_max) {
    sock_max = new_uart;
  } else if (client_sock > new_uart && client_sock > sock_max) {
    sock_max = client_sock;
  }

  /* add the sockets to the table */
  if (add_sockpair(new_uart, client_sock) < 0) {
    printf ("Socket table full\n");
    goto ACCEPT_FAILS;
  }

  /* add to fd set */
  FD_SET(new_uart, &sock_read);
  FD_SET(client_sock, &sock_read);

  printf ("Connections %d -> %d\n", new_uart, client_sock);
  return 1;

 ACCEPT_FAILS: {
    close_socks(new_uart, client_sock);
    return -1;
  }
}

int main(int argc, char *argv[])
{
  int flags;

  /* register at exit handler */
  if (atexit(exit_nice) < 0) {
    fprintf(stderr, "atexit failed\n");
    exit(1);
  }

  /* reg sig handlers */
  (void) signal(SIGHUP, sig_catch);
  (void) signal(SIGINT, sig_catch);
  (void) signal(SIGQUIT, sig_catch);
  (void) signal(SIGTERM, sig_catch);
  (void) signal(SIGPIPE, sig_pipe);
  
  /* init the global structs and arrays */
  init_structs();

  /* Create socket */
  if((accept_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket open failed");
    exit(1);
  }
  FD_SET(accept_sock, &sock_read);
  FD_SET(accept_sock, &sock_except);

  /* Bind to given port */
  if(bind(accept_sock, (struct sockaddr *) &uart_addr, sizeof(uart_addr)) < 0) {
    perror("Bind failed");
    exit(1);
  }
  listen(accept_sock, 5);
  flags = fcntl (accept_sock, F_GETFL); 
  flags |= O_NONBLOCK; 
  fcntl (accept_sock, F_SETFL, flags); 

  sock_max = accept_sock;

  /* Now the uart_sock is in the listen state, we can monitor it
   * for incomming connections. when a connection is accepted, the new
   * socket is added to the array */

  while (1) {
    int nevents;
    int i;

    nevents = select(sock_max + 1, &sock_read, &sock_write,
		     &sock_except, NULL);
    if (nevents <= 0) {
      perror("select");
      goto FD_READD;
    }
    
    /* see if this is a new connection on accept_sock */
    if (FD_ISSET(accept_sock, &sock_read)) {
      nevents--;
      accept_connect(accept_sock);
    }

    /* see if any other socket is ready for reading */
    while (nevents-- > 0) {
      int sock_index;
      int read_fd, write_fd;
      
      sock_index = get_next_fd(&read_fd, &write_fd);
      if (sock_index < 0) {
	printf ("Error for event %d\n", nevents + 1);
	goto FD_READD;
      }

      if (read_write(read_fd, write_fd) < 0) {
	/* close connection */
	del_sockindex(sock_index);
	goto FD_READD;
      }
    }

    /* add fds to fd set */
  FD_READD: {
      int count = 0;
      int *uart = sock_pair.uart;
      int *client = sock_pair.client;
	
      FD_ZERO(&sock_read);
      FD_SET(accept_sock, &sock_read);
      sock_max = accept_sock;
      for (i=0; i <MAX_CONN && count <= sock_pair.len; i++) {
	if (uart[i] != -1 && client[i] != -1) {
	  FD_SET(uart[i], &sock_read);
	  FD_SET(client[i], &sock_read);
	  
	  /* set the max sock count */
	  if (uart[i] > client[i] && uart[i] > sock_max) sock_max = uart[i];
	  else if (client[i] > uart[i] && client[i] > sock_max) sock_max = client[i];
	  count++;
	}
      }
      //printf ("Added total of %d FDs\n", 2*count+1);

    }
  }
}
