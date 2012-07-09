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
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

/*
 * the port on which we listen 
 */
#define SERVER_PORT 8765

// this is the size of a AM packet. bad things happen if this is not set
// properly
// to set this properly, you will have to determine it manually
// not just sizeof(AM_MSG) * 8 - this will not work because of bit padding
#define BUF_SIZE (662)

#define MAX_CONN 256
#include <sys/time.h>

/*
 * globals 
 */
/*
 * the topology array 
 */
char            topology[256][256];

struct {
    int             mote_sock[MAX_CONN];
    int             mote_id[MAX_CONN];
    int             len;	/* the two arrays are always equal */
} sock_pair;
int             accept_sock;

pthread_mutex_t big_mutex = PTHREAD_MUTEX_INITIALIZER;
#define BIG_LOCK pthread_mutex_lock(&big_mutex);
#define BIG_UNLOCK pthread_mutex_unlock(&big_mutex);

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
    BIG_LOCK;
    for (i = 0; i < MAX_CONN; i++) {
	if (sock_pair.mote_sock[i] == -1 && sock_pair.mote_id[i] == -1) {
	    sock_pair.mote_sock[i] = mote;
	    sock_pair.mote_id[i] = mote_id;
	    sock_pair.len++;
	    BIG_UNLOCK;
	    return i;
	}
    }
    BIG_UNLOCK;
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
     * close 
     */
    printf("Mote %d on sock %d disconnects\n", *mote_id, *mote);
    close(*mote);
    BIG_LOCK;
    *mote = *mote_id = -1;
    sock_pair.len--;
    BIG_UNLOCK;
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
read_write_all(int index, void *buf)
{
    int             r,
                    w,
                    i;
    int             mote_id;
    int             read_fd;

    // no need to lock since these objects are local to this thread
    mote_id = sock_pair.mote_id[index];
    read_fd = sock_pair.mote_sock[index];

    r = recv(read_fd, buf, BUF_SIZE, MSG_WAITALL);
    if (r < 0) {
	perror("read");
	return r;
    } else if (r == 0) {
	/*
	 * connection has terminated 
	 */
	return -1;
    }
    /*
     * this makes sure that we read one full packet at a time and write it 
     * to all listeners. make sure the BUF_SIZE is set to the AM_PACKET
     * size multiplied by 8 (see the Fullpc header for implementation) 
     */
    printf("read %d\n", r);
    if (r < BUF_SIZE) {
	// d ata is probably corrupted
	printf("FULL_PACKET_WRITE: Got %d when expecting %d\n", r,
	       BUF_SIZE);
	return -1;
    }


    /*
     * we have to write to every other sock in table 
     */
    printf("Writing data from Mote %d to ", mote_id);
    for (i = 0; i < MAX_CONN; i++) {
	BIG_LOCK;
	if (sock_pair.mote_sock[i] == -1 || sock_pair.mote_id[i] == -1) {
	    BIG_UNLOCK;
	    continue;
	}
	// we write to only those mote pairs that are in the topology
	// table
	if (!topology[mote_id][sock_pair.mote_id[i]]) {
	    BIG_UNLOCK;
	    continue;
	}

	printf("%d, ", sock_pair.mote_id[i]);
	w = write(sock_pair.mote_sock[i], buf, r);
	if (w != r) {
	    perror("write");
	    // return -1;
	}
	BIG_UNLOCK;
    }
    printf("\n");
    fflush(stdout);

    return 1;
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

/*
 * This function binds to specified port and creates a new
 * thread when accept() returns
 * The socket accept() returns (not the ptr) is passed to the
 * function.
 * Never returns unless there is an error or server is killed.
 */
void
bind_accept_fork(int bind_port, void *start_thread)
{

    int             newsockfd,
                    clilen;
    struct sockaddr_in cli_addr,
                    serv_addr;
    pthread_t       chld_thr;

    /*
     * Create socket 
     */
    if ((accept_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("Socket open failed");
	exit(1);
    }

    /*
     * Fill structures 
     */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(bind_port);

    /*
     * Bind to given port 
     */
    if (bind
	(accept_sock, (struct sockaddr *) &serv_addr,
	 sizeof(serv_addr)) < 0) {
	perror("Bind failed");
	exit(1);
    }
    listen(accept_sock, 5);

    for (;;) {
	clilen = sizeof(cli_addr);
	newsockfd =
	    accept(accept_sock, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd < 0) {
	    perror("Accept failed");
	    exit(1);
	}

	/*
	 * create a new thread to process the incomming request note: dont 
	 * pass ptr to newsockfd! 
	 */
	if (pthread_create(&chld_thr, NULL, start_thread,
			   (void *) newsockfd) != 0) {
	    perror("Thread create failed");
	    exit(1);
	}
	if (pthread_detach(chld_thr)) {
	    perror("Thread could not be detached");
	    exit(1);
	}

	/*
	 * the server is now free to accept another socket request 
	 */
    }
    return;
}

void           *
start_thread(void *args)
{
    int             mote_sock = (int) args;
    int             mote_id;
    int             index;
    void           *buf;

    /*
     * get mote id 
     */
    if (read(mote_sock, &mote_id, 4) < 0) {
	perror("read");
	close(mote_sock);
	pthread_exit(NULL);
    }
    printf("Got connection on %d from mote_id %d\n", mote_sock, mote_id);
    index = add_sockpair(mote_sock, mote_id);
    if (index < 0) {
	printf("Table is full\n");
	close(mote_sock);
	pthread_exit(NULL);
    }

    if ((buf = malloc(BUF_SIZE)) == NULL) {
	perror("malloc");
	close(mote_sock);
	pthread_exit(NULL);
    }

    while (1) {
	if (read_write_all(index, buf) < 0) {
	    break;
	}
    }

    free(buf);
    del_sockindex(index);
    return NULL;
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

    bind_accept_fork(SERVER_PORT, start_thread);
}
