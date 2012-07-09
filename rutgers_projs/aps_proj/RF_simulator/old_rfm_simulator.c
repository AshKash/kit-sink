/*
 * This is intended as a replacement of the Java program ConnectionManager
 * Whenever a simulated mote comes up, it connects to localhost:9876 and 
 * sends the TOS_LOCAL_ADDRESS as four bytes. After this, Every _bit_ sent
 * over the radio is sent as one _byte_ over the socket.
 * This server reads the initial four bytes and maps the socket to the 
 * mote id. The server uses select() and sends every byte it receives to
 * all other sockets, except the socket the data came on.
 * from the tests I have conducted this works much better than the 
 * ConnectionManager.java. For example, the APS code does not work with the
 * Java version, but does work with this.
 */

/*
 * Initial version: Ashwin (ashwink@paul.rutgers.edu) 
 */

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

/*
 * the port on which we listen 
 */
#define SERVER_PORT 9876

#define CLIENT_PORT 7654
#define CLIENT_NAME "localhost"

// this is the size of a AM packet. bad things happen if this is not set
// properly
// to set this properly, you will have to determine it manually
// not just sizeof(AM_MSG) * 8 - this will not work because of bit padding
#define BUF_SIZE (2102)

#define MAX_CONN 256
#define FD_SETSIZE (MAX_CONN)
#include <sys/time.h>

/*
 * globals 
 */
/*
 * the topology array 
 */
char            topology[256][256];

char            buf[BUF_SIZE];
fd_set          sock_read,
                sock_write,
                sock_except;
struct {
    int             mote_sock[MAX_CONN];
    int             mote_id[MAX_CONN];
    int             len;	/* the two arrays are always equal */
} sock_pair;
int             sock_max,
                accept_sock,
                null_sock;

struct sockaddr_in mote_addr,
                client_addr;

/*
 * closes sockets nicely 
 */
int
close_socks(int mote, int mote_id)
{
    /*
     * simple things... 
     */
    if (mote != -1) {
	close(mote);
    }

    return 1;
}

/*
 * adds given int pair into the sock_pair list. returns true/false 
 */
int
add_sockpair(int mote, int mote_id)
{
    int             i;

    /*
     * scan for next pos 
     */
    for (i = 0; i < MAX_CONN; i++) {
	if (sock_pair.mote_sock[i] == -1 && sock_pair.mote_id[i] == -1) {
	    sock_pair.mote_sock[i] = mote;
	    sock_pair.mote_id[i] = mote_id;
	    sock_pair.len++;
	    return 1;
	}
    }
    return -1;
}

/*
 * deletes the given index and does clean up 
 */
void
del_sockindex(int index)
{
    int            *mote,
                   *mote_id;
    mote = &sock_pair.mote_sock[index];
    mote_id = &sock_pair.mote_id[index];

    /*
     * check if sock exists 
     */
    if (*mote == -1 && *mote_id == -1)
	return;
    if (sock_pair.len <= 0) {
	fprintf(stderr, "sock_pair.len < 0!!\n");
	return;
    }

    /*
     * remove from fd set 
     */
    FD_CLR(*mote, &sock_read);
    FD_CLR(*mote, &sock_write);
    FD_CLR(*mote, &sock_except);
    FD_CLR(*mote_id, &sock_read);
    FD_CLR(*mote_id, &sock_write);
    FD_CLR(*mote_id, &sock_except);

    /*
     * close 
     */
    printf("Mote %d on sock %d disconnects\n", *mote_id, *mote);
    close_socks(*mote, *mote);
    *mote = *mote_id = -1;
    sock_pair.len--;
}

/*
 * opposite of add 
 */
int
del_sockpair(int mote, int mote_id)
{
    int             i;

    for (i = 0; i < MAX_CONN && sock_pair.len != 0; i++) {
	if (sock_pair.mote_sock[i] == mote
	    && sock_pair.mote_id[i] == mote_id) {
	    del_sockindex(i);
	    return 1;
	}

    }
    return -1;
}

/*
 * exit handler. closes all socks nicely before exiting 
 */
void
exit_nice(void)
{
    int             i;

    printf("Exiting...\n");

    shutdown(accept_sock, 2);
    close(accept_sock);
    for (i = 0; i < MAX_CONN && sock_pair.len != 0; i++) {
	del_sockindex(i);
    }
}

/*
 * catch sig pipe. this is mainly to prevent the process from dying on
 * SIGPIPE 
 */
void
sig_pipe(int sig)
{
    printf("SIGPIPE caught and ignored\n");
}


/*
 * catch signals 
 */
void
sig_catch(int sig)
{
    exit(0);
}

/*
 * inits all globals 
 */
void
init_structs(void)
{
    struct hostent *hp;
    int             i;

    /*
     * init the fd sets 
     */
    FD_ZERO(&sock_read);
    FD_ZERO(&sock_write);
    FD_ZERO(&sock_except);

    /*
     * init the sock structs 
     */
    memset((char *) &mote_addr, 0, sizeof(mote_addr));
    mote_addr.sin_family = AF_INET;
    mote_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mote_addr.sin_port = htons(SERVER_PORT);

    /*
     * client socket struct 
     */
    hp = gethostbyname(CLIENT_NAME);
    if (!hp) {
	printf("Can't get host entry for host %s\n", CLIENT_NAME);
	/*
	 * herror("gethostbyname"); 
	 */
	exit(-1);
    }
    memcpy(&client_addr.sin_addr, hp->h_addr,
	   sizeof(client_addr.sin_addr));
    client_addr.sin_port = ntohs(CLIENT_PORT);
    client_addr.sin_family = AF_INET;

    /*
     * fill the socket pair table 
     */
    sock_pair.len = 0;
    for (i = 0; i < MAX_CONN; i++) {
	sock_pair.mote_sock[i] = sock_pair.mote_id[i] = -1;
    }
}

/*
 * reads from one of the fds and writes data to the other. may block 
 */
int
read_write(int read_fd, int write_fd)
{
    int             r,
                    w,
                    i;
    int             mote_id;

    if (read_fd == null_sock) {
	fprintf(stderr, "FATAL!!!!\n");
	return -1;
    }
    buf = malloc(BUF_SIZE);
    r = recv(read_fd, buf, BUF_SIZE, MSG_WAITALL);
    /*
     * since sock is non blocking, this means some serious error 
     */
    if (r < 0) {
	perror("read");
	free(buf);
	return r;
    } else if (r == 0) {
	/*
	 * connection has terminated 
	 */
	free(buf);
	return -1;
    }
    /*
     * this makes sure that we read one full packet at a time and write it 
     * to all listeners. make sure the BUF_SIZE is set to the AM_PACKET
     * size multiplied by 8 (see the Fullpc header for implementation) 
     */
    /*
     * printf ("read %d\n", r); if (r < BUF_SIZE) {
     * printf("FULL_PACKET_WRITE: Got %d when expecting %d\n", r,
     * BUF_SIZE); return 0; } 
     */

    /*
     * find the mote_id responsible for this socket 
     */
    for (i = 0; i < MAX_CONN && sock_pair.len != 0; i++) {
	if (sock_pair.mote_sock[i] == read_fd) {
	    mote_id = sock_pair.mote_id[i];
	    break;
	}
    }

    /*
     * we have to write to every other sock in table 
     */
    printf("Writing data from Mote %d to ", mote_id);
    for (i = 0; i < MAX_CONN; i++) {
	if (sock_pair.mote_sock[i] == -1 || sock_pair.mote_id[i] == -1)
	    continue;

	// we write to only those mote pairs that are in the topology
	// table
	if (!topology[mote_id][sock_pair.mote_id[i]])
	    continue;

	write_fd = sock_pair.mote_sock[i];
	printf("%d, ", sock_pair.mote_id[i]);
	w = write(write_fd, buf, r);
	if (w != r) {
	    perror("write");
	    free(buf);
	    return -1;
	}
    }
    printf("\n");
    fflush(stdout);
    free(buf);

    return 1;
}

/*
 * this will return the index of the next available pair that can be read
 * it will also write the readable fd and the writable fds into the args 
 */
int
get_next_fd(int *read_fd, int *write_fd)
{
    int             i;
    int            *mote_fd = sock_pair.mote_sock;
    int            *mote_id_fd = sock_pair.mote_id;

    for (i = 0; i < MAX_CONN; i++) {
	if (mote_fd[i] == -1 || mote_id_fd[i] == -1)
	    continue;

	if (FD_ISSET(mote_fd[i], &sock_read)) {
	    FD_CLR(mote_fd[i], &sock_read);
	    *read_fd = mote_fd[i];
	    *write_fd = mote_id_fd[i];
	    return i;
	} else if (FD_ISSET(mote_id_fd[i], &sock_read)) {
	    FD_CLR(mote_id_fd[i], &sock_read);
	    *read_fd = mote_id_fd[i];
	    *write_fd = mote_fd[i];
	    return i;
	}
    }
    return -1;
}

/*
 * this will accept a connection on the given socket and connect the
 * socket to the other end 
 */
int
accept_connect()
{
    int             new_mote = -1;
    int             mote_id = -1;
    socklen_t       cli_len;

    cli_len = sizeof(mote_addr);
    new_mote = accept(accept_sock, (struct sockaddr *) &mote_addr,
		      &cli_len);
    if (new_mote < 0) {
	perror("accept");
	goto ACCEPT_FAILS;
    }

    /*
     * get mote id 
     */
    read(new_mote, &mote_id, 4);

    if (new_mote > sock_max) {
	sock_max = new_mote;
    }

    /*
     * add the sockets to the table 
     */
    if (add_sockpair(new_mote, mote_id) < 0) {
	printf("Socket table full\n");
	goto ACCEPT_FAILS;
    }

    /*
     * add to fd set 
     */
    FD_SET(new_mote, &sock_read);

    printf("Mote %d connects on socket %d\n", mote_id, new_mote);
    return 1;

  ACCEPT_FAILS:{
	close_socks(new_mote, new_mote);
	return -1;
    }
}

/*
 * reads the topology.txt file and adds connections accordingly 
 */
void
get_top()
{
    FILE           *top;
    int             i,
                    j;

    // fill the array with zeroes
    for (i = 0; i < 256; i++)
	for (j = 0; j < 256; j++) {
	    topology[i][j] = 0;
	}

    top = fopen("topology.txt", "r");
    if (top == NULL) {
	fprintf(stderr, "FIle open failed, assuming default topology\n");
	// fill arrays with ones
	for (i = 0; i < 256; i++)
	    for (j = 0; j < 256; j++) {
		topology[i][j] = !!(i - j);
	    }
	return;

    }

    while (fscanf(top, "%d %d", &i, &j) != EOF) {
	printf("%d,%d\n", i, j);
	topology[i][j] = topology[j][i] = 1;
    }
    fclose(top);
}

int
main(int argc, char *argv[])
{
    int             flags;

    /*
     * register at exit handler 
     */
    if (atexit(exit_nice) < 0) {
	fprintf(stderr, "atexit failed\n");
	exit(1);
    }

    /*
     * reg sig handlers 
     */
    (void) signal(SIGHUP, sig_catch);
    (void) signal(SIGINT, sig_catch);
    (void) signal(SIGQUIT, sig_catch);
    (void) signal(SIGTERM, sig_catch);
    (void) signal(SIGPIPE, sig_pipe);

    /*
     * init the global structs and arrays 
     */
    init_structs();
    get_top();

    /*
     * Create socket 
     */
    if ((accept_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("Socket open failed");
	exit(1);
    }
    FD_SET(accept_sock, &sock_read);
    FD_SET(accept_sock, &sock_except);

    /*
     * Bind to given port 
     */
    if (bind
	(accept_sock, (struct sockaddr *) &mote_addr,
	 sizeof(mote_addr)) < 0) {
	perror("Bind failed");
	exit(1);
    }
    listen(accept_sock, 5);
    flags = fcntl(accept_sock, F_GETFL);
    // flags |= O_NONBLOCK; 
    fcntl(accept_sock, F_SETFL, flags);

    sock_max = accept_sock;
    null_sock = open("/dev/null", O_WRONLY);
    if (null_sock < 0) {
	perror("open");
	exit(1);
    }
    printf("accept_sock: %d, null_sock: %d\n", accept_sock, null_sock);

    /*
     * Now the mote_sock is in the listen state, we can monitor it for
     * incomming connections. when a connection is accepted, the new
     * socket is added to the array 
     */

    while (1) {
	int             nevents;
	int             i;

	nevents = select(sock_max + 1, &sock_read, &sock_write,
			 &sock_except, NULL);
	if (nevents <= 0) {
	    perror("select");
	    goto FD_READD;
	}

	/*
	 * see if this is a new connection on accept_sock 
	 */
	if (FD_ISSET(accept_sock, &sock_read)) {
	    nevents--;
	    accept_connect(accept_sock);
	}

	/*
	 * see if any other socket is ready for reading 
	 */
	while (nevents-- > 0) {
	    int             sock_index;
	    int             read_fd,
	                    write_fd;

	    sock_index = get_next_fd(&read_fd, &write_fd);
	    if (sock_index < 0) {
		printf("Error for event %d\n", nevents + 1);
		goto FD_READD;
	    }

	    if (read_write(read_fd, write_fd) < 0) {
		/*
		 * close connection 
		 */
		del_sockindex(sock_index);
		goto FD_READD;
	    }
	}

	/*
	 * add fds to fd set 
	 */
      FD_READD:{
	    int             count = 0;
	    int            *mote = sock_pair.mote_sock;
	    int            *mote_id = sock_pair.mote_id;

	    FD_ZERO(&sock_read);
	    FD_SET(accept_sock, &sock_read);
	    sock_max = accept_sock;
	    for (i = 0; i < MAX_CONN && count <= sock_pair.len; i++) {
		if (mote[i] != -1) {
		    FD_SET(mote[i], &sock_read);

		    /*
		     * set the max sock count 
		     */
		    sock_max = mote[i];
		    count++;
		}
	    }
	    // printf ("Added total of %d FDs\n", 2*count+1);

	}
    }
}
