#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"

#define BAD_SOCKET (-1)
#define ADDR_LOCK pthread_mutex_lock(&addr_lock);
#define ADDR_UNLOCK pthread_mutex_unlock(&addr_lock);

/*
 * Serialize access to the address table 
 */
pthread_mutex_t addr_lock = PTHREAD_MUTEX_INITIALIZER;

extern int      errno;
int             addr_table[TABLE_SIZE];	/* mote id - socket mappings */

/*
 ************* Globals *************
 */
int             server_port = 0;
int             need_to_sync = 0;
pthread_mutex_t serial_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * called when C-z is pressed 
 */
void
sigstop_handle(int a)
{
    need_to_sync = 1;
}


/*
 * Main program
 */
int
main(int argc, char **argv)
{
    int             i;
    pthread_t       uart_thr;

    if (argc < 2) {
	printf("Error! Give the port to bind to\n");
	exit(1);
    }
    sscanf(argv[1], "%d", &server_port);

    /*
     * Uart init 
     */
    if (uart_init() < 0) {
	printf("Sorry, Uart failed to init. Exiting...\n");
	exit(1);
    }

    /*
     * Init the addr table 
     */
    for (i = 0; i < TABLE_SIZE; i++)
	addr_table[i] = BAD_SOCKET;

    /*
     * Thread that listens on the uart 
     */
    if (pthread_create(&uart_thr, NULL, uart_listener, NULL) != 0) {
	perror("uart_listener thread");
	exit(1);
    }
    pthread_detach(uart_thr);
    printf("Create listener, LWP %ld\n", uart_thr);

    /*
     * Call the listener/forker 
     */
    printf("Binding to port %d...\n", server_port);
    bind_accept_fork(server_port, do_chld);

    /*
     * Never reached 
     */
    printf("FATAL!!! Server has crashed\n");
    return (0);
}

/*
 * This is the routine that is executed from a new thread 
 * The thread represents one client, and is spawned when a new
 * connection is made.
 */

void           *
do_chld(void *arg)
{
    int             cl_sock = (int) arg;
    TOS_Msg         am_pkt;

    printf("Child thread: Socket number = %d\n", cl_sock);

    /*
     * The first packet must be the am_reg pkt, else close connection 
     */
    if (am_client_reg(cl_sock) < 0) {
	printf("Client Failed to register, killing...\n");
	goto END;
    }

    while (1) {
	/*
	 * receive the packet and transmit over the serial 
	 */
	if (recv_tos_msg(cl_sock, &am_pkt) < 0)
	    goto END;

	/*
	 * send it over the serial 
	 */
	printf("Send ");
	print_packet(&am_pkt);
	if (need_to_sync) {
	    // cant really do any thing except discard the uart buffs
	    // also proper sync still needs to be done
	    printf("Sync from send\n");
	    uart_drain();
	}
	if (send_uart(&am_pkt) < 0)
	    goto END;
    }

  END:
    /*
     * close the socket and exit this thread 
     */
    am_client_dereg(cl_sock);
    printf("Child thread exiting\n");
    pthread_exit((void *) NULL);

    return (NULL);
}


/*
 * This function is called when the type of the AM is AM_CLIENT_REG It
 * will associate the address contained within the AM with the socket that 
 * the message was received on 
 */
int
am_client_reg(int socket)
{
    TOS_Msg         am_reg;
    char            mote_id;

    /*
     * Get the message from the socket 
     */
    if (recv_tos_msg(socket, &am_reg) < 0)
	return -1;

    if (am_reg.type != (char) AM_CLIENT_REG)
	return -1;

    mote_id = am_reg.data[0];
    /*
     * See if this id is already registered 
     */
    ADDR_LOCK;
    if (addr_table[(int) mote_id] != BAD_SOCKET) {
	ADDR_UNLOCK;
	return -1;
    }

    /*
     * echo back the message to client 
     */
    if (send_tos_msg(socket, &am_reg) < 0) {
	ADDR_UNLOCK;
	return -1;
    }

    /*
     * Update table 
     */
    addr_table[(int) mote_id] = socket;
    ADDR_UNLOCK;

    printf("Client on sock %d registered, id is %d\n", socket, mote_id);
    return 1;
}


/*
 * Deregister a client 
 */
void
am_client_dereg(int socket)
{
    int             mote_id;
    close(socket);

    ADDR_LOCK;
    mote_id = get_mote_id(socket);
    if (mote_id > -1 && mote_id < TABLE_SIZE)
	addr_table[mote_id] = BAD_SOCKET;
    ADDR_UNLOCK;

    printf("Client on sock %d Exited, id was %d\n", socket, mote_id);
}

/*
 * Reverse lookup the addr_table Caller must lock the table 
 */
int
get_mote_id(int socket)
{
    int             i;

    for (i = 0; i < TABLE_SIZE; i++)
	if (addr_table[i] == socket)
	    return i;

    printf("Error!!! socket not present in table\n");
    return -1;
}



/*
 * This function will read a AM packet from the serial and send the
 * packet to all appropriate clients Locks and unlocks the addr_table as
 * needed 
 */
void           *
uart_listener(void *args)
{
    TOS_Msg         am_pkt;
    int             i,
                    cl_sock;

    /*
     * signal stuff 
     */
    signal(SIGHUP, sigstop_handle);

    while (1) {
	if (need_to_sync) {
	    printf("calling Sync\n");
	    uart_sync();
	    need_to_sync = 0;
	}

	/*
	 * read pkt from uart 
	 */
	if (recv_uart(&am_pkt) < 0) {
	    perror("Error while reading the serial port!!!");
	}

	if ((int) am_pkt.addr == (int) TOS_BCAST_ADDR) {
	    /*
	     * Send it to all valid sockets 
	     */
	    printf("Recv ");
	    print_packet(&am_pkt);
	    ADDR_LOCK;
	    printf("sending to: ");
	    for (i = 0; i < TABLE_SIZE; i++) {
		if ((cl_sock = addr_table[i]) == BAD_SOCKET) {
		    continue;
		}
		if (send_tos_msg(cl_sock, &am_pkt) < 0)
		    printf("BCAST: Client on socket %d, id %d is dead\n",
			   cl_sock, i);
		else
		    printf("%d,", i);
	    }
	    printf("\n");
	    ADDR_UNLOCK;

	} else {
	    /*
	     * Send it to concerned mote socket 
	     */
	    printf("Recv2 ");
	    print_packet(&am_pkt);
	    cl_sock = addr_table[(int) am_pkt.addr];
	    if (cl_sock == BAD_SOCKET)
		continue;
	    if (send_tos_msg(cl_sock, &am_pkt) < 0)
		printf("Client on socket %d, id %d is dead\n", cl_sock,
		       am_pkt.addr);
	}
    }
}

/*
 * This function binds to specified port and creates a new
 * thread when accept() returns
 * The socket accept() returns (not the ptr) is passed to the
 * function.
 * Never returns unless there is an error or server is killed.
 */
void
bind_accept_fork(int bind_port, void *(*start_thread) (void *))
{

    int             sockfd,
                    newsockfd,
                    clilen;
    struct sockaddr_in cli_addr,
                    serv_addr;
    pthread_t       chld_thr;
    pthread_attr_t  thr_attr;

    /*
     * Create socket 
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
	0) {
	perror("Bind failed");
	exit(1);
    }
    listen(sockfd, 5);

    /*
     * Set the thread attributes 
     */
    pthread_attr_init(&thr_attr);
    pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_DETACHED);

    for (;;) {
	int             rbuff_size = 128;
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	/*
	 * Keep a very low recv buff due to slow speed interfaces on other 
	 * side 
	 */
	if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVBUF,
		       (void *) &rbuff_size, sizeof(int)) < 0) {
	    printf("setsockopt failed\n");
	    exit(1);
	}


	if (newsockfd < 0) {
	    perror("Accept failed");
	    exit(1);
	}

	/*
	 * create a new thread to process the incomming request note: dont 
	 * pass ptr to newsockfd! 
	 */
	if (pthread_create(&chld_thr, &thr_attr, start_thread,
			   (void *) newsockfd) != 0) {
	    perror("Thread create failed");
	    exit(1);
	}

	/*
	 * the server is now free to accept another socket request 
	 */
    }

    /*
     * Never reached 
     */
    return;
}
