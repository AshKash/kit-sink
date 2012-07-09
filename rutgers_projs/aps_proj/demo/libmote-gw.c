#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include "mote-gw.h"
extern int      errno;

#define BAD_TYPE NULL
#define TYPE_LOCK pthread_mutex_lock(&type_lock);
#define TYPE_UNLOCK pthread_mutex_unlock(&type_lock);

/*
 * lock for gethostbyname 
 */
pthread_mutex_t h_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * lock to protect the type table 
 */
pthread_mutex_t type_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * The type table - holds pointers to function that takes TOS_Msg* and
 * returns TOS_Msg* Size of table is TABLE_SIZE - whew!! 
 */
TOS_Msg        *(*type_table[TABLE_SIZE]) (TOS_Msg *);

/*
 * connects to a server and returns the socket
 * takes the host name and the port
 */
int
connect_to_server(const char *hostname, int port_number)
{
    int             sock,
                    ret;
    struct sockaddr_in server;
    struct hostent *hp;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	perror("connect_to_server");
	return sock;
    }

    pthread_mutex_lock(&h_lock);
    hp = gethostbyname(hostname);
    pthread_mutex_unlock(&h_lock);
    if (!hp) {
	printf("Can't get host entry for host %s\n", hostname);
	/*
	 * herror("gethostbyname"); 
	 */
	exit(-1);
    }

    memcpy(&server.sin_addr, hp->h_addr, sizeof(server.sin_addr));
    server.sin_port = ntohs(port_number);
    server.sin_family = AF_INET;

    ret = connect(sock, (struct sockaddr *) &server, sizeof(server));
    if (ret < 0) {
	perror("connect_to_server");
	return ret;
    }

    return sock;
}



/*
 * Client initialization routine. Must be called by all clients before
 * starting communications. The server_socket will be returned, which the
 * client can make use for sending and receiving queries. The function can 
 * only be called once during client startup 
 */
int
client_init(int argc, char **argv)
{
    char           *mote_gw_name = getenv("MOTE_GW_NAME");
    char           *_port = getenv("MOTE_GW_PORT");
    char           *_mote_id = getenv("MOTE_ID");

    int             mote_gw_port;
    char            mote_id;
    int             gw_socket;
    TOS_Msg         reg_msg,
                    ret_msg;
    int             sbuff_size = 128;
    int             i;
    pthread_t       type_thr;

    /*
     * Try the command line if envp was bad 
     */
    if (mote_gw_name == NULL) {
	if (argc < 2)
	    usage();
	mote_gw_name = argv[1];
    }

    if (_port == NULL) {
	if (argc < 3)
	    usage();
	mote_gw_port = atoi(argv[2]);
    } else {
	mote_gw_port = atoi(_port);
    }

    if (_mote_id == NULL) {
	int             _id;
	if (argc < 4 || (_id = atoi(argv[3])) > 0xFF)
	    usage();
	mote_id = (char) _id;
    } else {
	mote_id = (char) atoi(_mote_id);
    }

    /*
     * Connect to server 
     */
    gw_socket = connect_to_server(mote_gw_name, mote_gw_port);
    if (gw_socket < 0)
	goto CLIENT_INIT_ERR;

    /*
     * Keep a very low send buff due to slow speed interfaces on other
     * side 
     */
    if (setsockopt(gw_socket, SOL_SOCKET, SO_SNDBUF,
		   (void *) &sbuff_size, sizeof(int)) < 0) {
	printf("setsockopt failed\n");
	exit(1);
    }

    /*
     * Register this client with the server 
     */
    reg_msg.type = AM_CLIENT_REG;	/* Mote-gw Reserved handler */
    reg_msg.data[0] = mote_id;
    send_tos_msg(gw_socket, &reg_msg);
    recv_tos_msg(gw_socket, &ret_msg);

    /*
     * See is everthing is ok 
     */
    if (compare_tos_msg(&ret_msg, &reg_msg) != 0) {
	printf("Mote ID must be unique\n");
	goto CLIENT_INIT_ERR;
    }

    /*
     * Init the type table 
     */
    TYPE_LOCK;
    for (i = 0; i < TABLE_SIZE; i++)
	type_table[i] = BAD_TYPE;
    TYPE_UNLOCK;

    /*
     * Start the listener thread 
     */
    if (pthread_create(&type_thr, NULL, type_listener, (void *) gw_socket)
	!= 0) {
	printf("Thread create failed\n");
	return -1;
    }
    pthread_detach(type_thr);

    return gw_socket;

  CLIENT_INIT_ERR:
    printf("Client failed initialization...\n");
    exit(1);
}


/*
 * Sends the given TOS msg over the socket. Returns -1 on failure 
 */
int
send_tos_msg(int socket, TOS_Msg * tos_msg_ptr)
{
    int             write_bytes;
    int             msg_size = AM_PKT_LEN;

    tos_msg_ptr->group = LOCAL_GROUP;
    write_bytes = write(socket, tos_msg_ptr, msg_size);
    if (write_bytes != msg_size)
	return -1;

    return 1;
}

/*
 * Receives a TOS msg over the socket and stores it 
 */
int
recv_tos_msg(int socket, TOS_Msg * tos_msg_ptr)
{
    int             bytes_read = read_all(socket, tos_msg_ptr, AM_PKT_LEN);

    if (bytes_read != AM_PKT_LEN) {
	printf("recv_tos_msg: read_all failed\n");
	return -1;
    }
    return bytes_read;
}

/*
 * Registers the am type with a handler. Whenever a AM is received with
 * the specified type, the registered handler will be called All other
 * types of AMs will be dropped The function must return the processed
 * packet if it needs to be transmitted back, or a NULL to drop the
 * packet 
 */
int
reg_am_type(char type, TOS_Msg * am_handler(TOS_Msg * am_pkt))
{
    /*
     * Add the handler to the type table if one is not already present 
     */
    TYPE_LOCK;
    if (type_table[(int) type] != BAD_TYPE) {
	TYPE_UNLOCK;
	return -1;
    }
    type_table[(int) type] = am_handler;
    TYPE_UNLOCK;

    return 1;
}

/*
 * Deregister the handler 
 */
int
dereg_am_type(char type)
{
    TYPE_LOCK;
    type_table[(int) type] = BAD_TYPE;
    TYPE_UNLOCK;

    return 1;
}

/*
 * Reads on a socket and receives all TOS_Msg packets. It then calls the
 * appropriate handler for the type 
 */
void           *
type_listener(void *arg)
{
    int             gw_socket = (int) arg;
    TOS_Msg         am_pkt;
    TOS_Msg        *am_ptr;

    printf("Listening on socket %d\n", gw_socket);

    while (1) {
	if (recv_tos_msg(gw_socket, &am_pkt) < 0) {
	    /*
	     * Server probably is dead 
	     */
	    printf("Error while receiving packet, Exiting...\n");
	    exit(1);
	}

	/*
	 * Call the appropriate handler 
	 */
	TYPE_LOCK;
	if (type_table[(int) am_pkt.type] != BAD_TYPE) {
	    am_ptr = type_table[(int) am_pkt.type] (&am_pkt);
	    if (am_ptr != NULL) {
		/*
		 * Unlock before send 
		 */
		TYPE_UNLOCK;
		/*
		 * Transmit 
		 */
		if (send_tos_msg(gw_socket, am_ptr) < 0) {
		    /*
		     * Server is probably dead 
		     */
		    printf
			("Error while trying to send packet. Exiting...\n");
		    exit(1);
		}
	    } else
		TYPE_UNLOCK;
	} else
	    TYPE_UNLOCK;
    }

    /*
     * Never reached 
     */
    return NULL;
}

/*
 * Reads upto buff_size or dies on error. Returns number of bytes read 
 */
int
read_or_die(int socket, void *buff, size_t buff_size)
{
    int             ret;
    if ((ret = read(socket, (void *) (buff), buff_size)) < 0) {
	perror("Read on socket failed");
	close(socket);
	exit(1);
	return (ret);
    }
    return (ret);
}

/*
 * Keeps reading until the needed number of bytes are received returns the 
 * actual number of bytes read 
 */
int
read_all(int socket, void *buff, size_t buff_size)
{
    int             tmp = 0;
    int             bytes_read = 0;

    do {
	tmp = read(socket, buff + bytes_read, buff_size - bytes_read);
	if (tmp <= 0) {
	    return (bytes_read);
	}
	bytes_read += tmp;
    } while (bytes_read < buff_size);
    return bytes_read;
}

/*
 * Keeps reading until the needed number bytes are received 
 */
int
read_all_or_die(int socket, void *buff, size_t buff_size)
{

    int             bytes_read = read_all(socket, buff, buff_size);
    if (bytes_read != buff_size)
	exit(1);

    return bytes_read;
}

/*
 * Writes given number of bytes or dies on error 
 */
int
write_or_die(int socket, void *buff, size_t buff_size)
{
    int             ret;
    if ((ret = write(socket, (void *) (buff), buff_size)) != buff_size) {
	perror("Write on socket failed");
	close(socket);
	exit(1);
    }
    return (ret);
}

/*
 * Compares two TOS_Msg and returns 0 if equal 
 */
int
compare_tos_msg(TOS_Msg * a, TOS_Msg * b)
{
    int             i;

    if (a->addr != b->addr || a->type != b->type || a->group != b->group)
	return -1;

    for (i = 0; i < sizeof(a->data); i++)
	if (a->data[i] != b->data[i])
	    return -1;

    return 0;
}

/*
 * Client Usage 
 */
void
usage(void)
{
    printf("Usage:\n");
    printf("./client [<Mote GW name/IP> <Mote GW port> <Mote ID>]\n");
    printf("OR export MOTE_GW_NAME, MOTE_GW_PORT and MOTE_ID\n");
    exit(1);
}


/*
 * prints the am packet 
 */
void
print_packet(TOS_Msg * am_pkt)
{
    // now print out the packet and write it to a file.
    int             i;
    char           *pkt = (char *) am_pkt;

    printf("data:");
    for (i = 0; i < AM_PKT_LEN; i++) {
	printf("%x,", pkt[i] & 0xff);
    }
    printf("\n");
}
