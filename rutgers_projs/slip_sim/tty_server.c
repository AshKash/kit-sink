#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <pty.h>
#include <termios.h>

/* the port on which we listen */
#define SERVER_PORT 7654

#define BUF_SIZE (1024 * 1024)
#define PATH_BUF (256)

#define MAX_CONN 1024
#define NR_TH 4           /* number of concurrent threads */
#define IFCONFIG "/sbin/ifconfig"

/* globals */
char buf[BUF_SIZE];
fd_set sock_read, sock_write, sock_except;

/* even though we have the stty, we dont care about it. the stty fd simply
 * belongs to the kernel slip to read/write into. we recv/send all data on
 * the corresponding mtty. however when a conn goes down, the triplets need
 * to be closed. also since we are using virtual ttys, state is not restored!
 * when we close stty, we also have to ifdown the corresponding iface */
struct {
  int server[MAX_CONN];    /* socket */
  int mtty[MAX_CONN];      /* master tty */
  int stty[MAX_CONN];      /* slave tty */
  char stty_if_name[MAX_CONN][PATH_BUF];   /* name of the interface */
  int len;    /* the two arrays are always equal */
} sock_pair;
int sock_max, accept_sock;
char local_ip[PATH_BUF];

struct sockaddr_in server_addr, mtty_addr;

/****************** copied from slattach **********************/

/* Fetch the name of the network interface attached to this terminal. */
static int
tty_get_name(int tty_fd, char *name)
{
  int r = ioctl(tty_fd, SIOCGIFNAME, name);
  if (r < 0) {
    perror("tty_get_name");
    return(-errno);
  }
  return(0);
}

/* Set the line discipline of a terminal line. */
static int SLIP_set_disc(int fd, int disc)
{
  if (ioctl(fd, TIOCSETD, &disc) < 0) {
    fprintf(stderr, "SLIP_set_disc(%d): %s\n", disc, strerror(errno));
    return (-errno);
  }
  return (0);
}

/* Set the encapsulation type of a terminal line. */
static int SLIP_set_encap(int fd, int encap)
{
  if (ioctl(fd, SIOCSIFENCAP, &encap) < 0) {
    fprintf(stderr, "SLIP_set_encap(%d): %s\n", encap, strerror(errno));
    return (-errno);
  }
  return (0);
}

/* deactivate */
int SLIP_deactivate(int index)
{
  char args_buf[PATH_BUF];
  char *iface_name = sock_pair.stty_if_name[index];

  sprintf(args_buf, IFCONFIG " %s down", iface_name);
  printf("Running %s\n", args_buf);

  if (system(args_buf) < 0) {
    perror("system");
    return -1;
  }

  return 1;

}

/* Start the SLIP encapsulation on the file descriptor. */
int SLIP_activate(int index)
{
  char *iface_name = sock_pair.stty_if_name[index];
  char args_buf[PATH_BUF];
  char remote_ip[16];
  int last_dig, last2_dig;
  int fd = sock_pair.stty[index];

  if (SLIP_set_disc(fd, N_SLIP) < 0)
    return (-1);
  if (SLIP_set_encap(fd, 0) < 0)
    return (-1);

  /* do ifconfig slN here */
  if (tty_get_name(fd, iface_name) < 0) {
    return -1;
  }
  sscanf(iface_name, "sl%d", &last_dig);
  last2_dig = last_dig / 255;
  last_dig = last_dig % 255;
    
  sprintf(remote_ip, "10.1.%d.%d", last2_dig, last_dig+1);  
  sprintf(args_buf, IFCONFIG " %s %s pointopoint %s", iface_name,
	  local_ip, remote_ip);
  printf ("Running: %s\n", args_buf);

  /* doit */
  if (system(args_buf) < 0) {
    perror("system");
    return -1;
  }
  
  return (0);
}

/* Start the VJ-SLIP encapsulation on the file descriptor. */
int CSLIP_activate(int fd)
{
  if (SLIP_set_disc(fd, N_SLIP) < 0)
    return (-1);
  if (SLIP_set_encap(fd, 1) < 0)
    return (-1);
  return (0);
}

/********************** slattach ********************/

/* closes the server socket, the mtty and the stty. does special cleanup for stty */
int close_socks(int index)
{
  int server_sock = sock_pair.server[index];
  int mtty = sock_pair.mtty[index];
  int stty = sock_pair.stty[index];

  /* simple things... */
  if (server_sock != -1) {
    close(server_sock);
  }

  if (mtty != -1) {
    close(mtty);
  }

  /* the hairy part */
  if (stty != -1) {

    /* ifdown iface */
    if (SLIP_deactivate(index) <0) {
      fprintf(stderr, "WARNING!!! SLIP_deactivate failed\n");
    }
    close(stty);
  }
  return 1;
}

/* adds given int pair into the sock_pair list. returns index pos or false */
int add_sockpair(int server, int mtty, int stty)
{
  int i;

  /* scan for next pos */
  for (i=0; i<MAX_CONN; i++) {
    if (sock_pair.server[i] == -1 &&
	sock_pair.mtty[i] == -1 &&
	sock_pair.stty[i] == -1) {
      sock_pair.server[i] = server;
      sock_pair.mtty[i] = mtty;
      sock_pair.stty[i] = stty;
      sock_pair.len++;
      return i;
    }
  }
  return -1;
}

/* deletes the given index and does clean up */
void del_sockindex(int index)
{
  int *server, *mtty, *stty;
  server = &sock_pair.server[index];
  mtty = &sock_pair.mtty[index];
  stty = &sock_pair.stty[index];

  /* check if sock exists */
  if (*server == -1 &&
      *mtty == -1 &&
      *stty == -1)
    return;
  if (sock_pair.len <= 0) {
    fprintf(stderr, "sock_pair.len < 0!!\n");
    return;
  }

  /* remove from fd set  - stty is ignored since it was never added */
  FD_CLR(*server, &sock_read);
  FD_CLR(*server, &sock_write);
  FD_CLR(*server, &sock_except);
  FD_CLR(*mtty, &sock_read);
  FD_CLR(*mtty, &sock_write);
  FD_CLR(*mtty, &sock_except);

  /* close */
  printf("Close: %d -> [%d, %d]\n", *server, *mtty, *stty);
  close_socks(index);
  *server = *mtty = *stty = -1;
  sock_pair.len--;
}

/* opposite of add */
int del_sockpair(int server, int mtty)
{
  int i;

  for (i=0; i<MAX_CONN && sock_pair.len != 0; i++) {
    if (sock_pair.server[i] == server && sock_pair.mtty[i] == mtty) {
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
  int i;

  /* init the fd sets */
  printf("FD_SETSIZE = %d\n", FD_SETSIZE);
  FD_ZERO(&sock_read);
  FD_ZERO(&sock_write);
  FD_ZERO(&sock_except);

  /* init the sock structs */
  memset((char *) &server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(SERVER_PORT);
  
  /* fill the socket pair table */
  sock_pair.len = 0;
  for (i=0; i<MAX_CONN; i++) {
    sock_pair.server[i] = sock_pair.mtty[i] = sock_pair.stty[i] = -1;
  }
}

/* reads from one of the fds and writes data to the other. may block */
int read_write(int read_fd, int write_fd)
{
  int r, w;

  r = read(read_fd, buf, BUF_SIZE);
  if (r < 0) {
    /* since sock is non blocking, this means some serious error */
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

/* this will return the index of the current pos that can be read
 * it will also write the readable fd and the writable fds into this pos in table */
int get_next_fd (int *read_fd, int *write_fd)
{
  int i;
  int *server_fd = sock_pair.server;
  int *mtty_fd = sock_pair.mtty;
  
  for (i = 0; i < MAX_CONN; i++) {
    if (server_fd[i] == -1 || mtty_fd[i] == -1) continue;

    if (FD_ISSET(server_fd[i], &sock_read)) {
      FD_CLR(server_fd[i], &sock_read);
      *read_fd = server_fd[i];
      *write_fd = mtty_fd[i];
      return i;
    } else if (FD_ISSET(mtty_fd[i], &sock_read)) {
      FD_CLR(mtty_fd[i], &sock_read);
      *read_fd = mtty_fd[i];
      *write_fd = server_fd[i];
      return i;
    }
  }
  return -1;
}

/* gets local ip and copies to local_ip */
void get_set_ip(void)
{
  char hostname[1024];
  struct hostent *hp;

  /* who are we? */ 
  if (gethostname(hostname, sizeof(hostname) < 0)) {
    perror("gethostname");
    strcpy(hostname, "mobile-gw-6.rutgers.edu");
    //return -1;
  }
  
  hp = gethostbyname(hostname);
  if (!hp) {
    fprintf(stderr, "gethostbyname() failed\n");
    strncpy(local_ip, "128.6.157.136", sizeof(local_ip));
  } else {
    strncpy(local_ip, inet_ntoa(*((struct in_addr*)hp->h_addr)),
	    sizeof(local_ip));
  }
}


/* this will accept a connection on the given socket and connect the socket
 * to the other end */
int accept_connect()
{
  int new_server = -1;
  int mtty_sock = -1, stty_sock = -1;
  socklen_t  cli_len;
  char slavename[1024];
  struct termios tios;
  int index = -1;

  /* server_sock is assumed to be non blocking */
  cli_len = sizeof(server_addr);
  new_server = accept(accept_sock, (struct sockaddr*) &server_addr,
		    &cli_len);
  if (new_server < 0) {
    perror ("accept");
    goto ACCEPT_FAILS;
  }

  /* connect to other end - /dev/ptmx*/
  if (openpty(&mtty_sock, &stty_sock, slavename, NULL, NULL)) {
    perror("openpty");
    close (mtty_sock);
    close(stty_sock);
    goto ACCEPT_FAILS;
  }

  /* make the tty raw */
  if (tcgetattr(mtty_sock, &tios) < 0) {
    perror("tcgetattr");
    goto ACCEPT_FAILS;
  }
  cfmakeraw(&tios);
  if (tcsetattr(mtty_sock, TCSANOW, &tios) < 0) {
    perror("tcsetattr");
    goto ACCEPT_FAILS;
  } 

  /* raw tty for slave */
  if (tcgetattr(stty_sock, &tios) < 0) {
    perror("tcgetattr");
    goto ACCEPT_FAILS;
  }
  cfmakeraw(&tios);
  if (tcsetattr(stty_sock, TCSANOW, &tios) < 0) {
    perror("tcsetattr");
    goto ACCEPT_FAILS;
  }      

  /* add the sockets to the table - must be done befor slip_activate */
  index = add_sockpair(new_server, mtty_sock, stty_sock);
  if (index < 0) {
    fprintf (stderr, "Socket table full\n");
    goto ACCEPT_FAILS;
  }

  /* activate slave as the slip interface */
  printf("master on %d, Slave is %s\n", mtty_sock, slavename);
  if (SLIP_activate(index) < 0) {
    fprintf(stderr, "SLIP activation failed\n");
    goto ACCEPT_FAILS;

  }

  if (new_server > mtty_sock && new_server > sock_max) {
    sock_max = new_server;
  } else if (mtty_sock > new_server && mtty_sock > sock_max) {
    sock_max = mtty_sock;
  }

  /* add to fd set - stty is ignored */
  FD_SET(new_server, &sock_read);
  FD_SET(mtty_sock, &sock_read);

  printf ("Connections %d -> [%d, %d]\n", new_server, mtty_sock, stty_sock);
  return 1;

 ACCEPT_FAILS: {
    if (index != -1)
      close_socks(index);
    return -1;

  }
}

/* the main loop - can be a thread */
void *main_loop(void* arg)
{
  while (1) {
    int nevents;
    int i;

    nevents = select(sock_max + 1, &sock_read, NULL,
		     NULL, NULL);
    if (nevents <= 0) {
      perror("select");
      goto FD_READD;
    }
    
    /* see if this is a new connection on accept_sock */
    if (FD_ISSET(accept_sock, &sock_read)) {
      nevents--;
      accept_connect(accept_sock);
      /* flow thru even if accept_connect fails */
    }

    /* see if any other socket is ready for reading */
    while (nevents-- > 0) {
      int sock_index;
      int read_fd, write_fd;
      
      sock_index = get_next_fd(&read_fd, &write_fd);
      if (sock_index < 0) {
	fprintf (stderr, "Error for event %d\n", nevents + 1);
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
      int *server = sock_pair.server;
      int *mtty = sock_pair.mtty;

      FD_ZERO(&sock_read);
      FD_SET(accept_sock, &sock_read);
      sock_max = accept_sock;
      for (i=0; i <MAX_CONN && count <= sock_pair.len; i++) {
	
	if (server[i] != -1 && mtty[i] != -1) {
	  FD_SET(server[i], &sock_read);
	  FD_SET(mtty[i], &sock_read);

	  /* set the max sock count */
	  if (server[i] > mtty[i] && server[i] > sock_max) sock_max = server[i];
	  else if (mtty[i] > server[i] && mtty[i] > sock_max) sock_max = mtty[i];
	  count++;
	}
      }
      //printf ("Added total of %d FDs\n", 2*count+1);
    }
  }
}

int main(int argc, char *argv[])
{
  int flags;

  /* register at exit */
  if (atexit(exit_nice) < 0) {
    fprintf(stderr, "atexit() failed\n");
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
  get_set_ip();

  /* Create socket */
  if((accept_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket open failed");
    exit(1);
  }
  FD_SET(accept_sock, &sock_read);
  FD_SET(accept_sock, &sock_except);

  /* Bind to given port */
  if(bind(accept_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    exit(1);
  }
  listen(accept_sock, 5);
  flags = fcntl (accept_sock, F_GETFL); 
  flags |= O_NONBLOCK; 
  fcntl (accept_sock, F_SETFL, flags); 
  
  sock_max = accept_sock;

  /* Now the accept_sock is in the listen state, we can monitor it
   * for incomming connections. when a connection is accepted, the new
   * socket is added to the mtty_sock array */
  main_loop(NULL);

  /* never reached */
  return 1;
}
