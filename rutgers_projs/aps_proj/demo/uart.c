#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
// #include <error.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include "timing.h"
#include "mote-gw.h"

#define BAUDRATE B19200

// delay b/w pkt xmits
#define COMM_SEND_DELAY (200 * 1000)

int             comm_sock = -1;

/*
 * Flow control lock 
 */
pthread_mutex_t flow_lock = PTHREAD_MUTEX_INITIALIZER;
#define FLOW_LOCK  /*printf("flow lock on %d \n", __LINE__);*/ pthread_mutex_lock(&flow_lock);
#define FLOW_UNLOCK /*printf("flow unlock on %d\n", __LINE__);*/ pthread_mutex_unlock(&flow_lock);
#define TRY_FLOW_LOCK /*printf("flow trylock on %d\n", __LINE__);*/ pthread_mutex_trylock(&flow_lock);

/*
 * Initialize the UART 
 */
int
uart_init(void)
{

    struct termios  oldtio,
                    newtio;

    if (comm_sock > 0) {
	printf("Init already done\n");
	return 1;
    }

    /*
     * open com1 for read/write 
     */
    comm_sock = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_SYNC);
    if (comm_sock == -1) {
	perror(": com1 open fails\n");
	return -1;
    }
    printf("com1 open ok\n");

    /*
     * save old serial port setting 
     */
    tcgetattr(comm_sock, &oldtio);
    memset(&newtio, 0, sizeof(newtio));

    // newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    /*
     * newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD; newtio.c_iflag =
     * IGNPAR | ICRNL; newtio.c_lflag = ICANON; //raw o/p newtio.c_oflag = 
     * 0; 
     */
    newtio = oldtio;
    cfmakeraw(&newtio);

    tcflush(comm_sock, TCIOFLUSH);
    tcdrain(comm_sock);
    tcsetattr(comm_sock, TCSANOW, &newtio);

    /*
     * return 
     */
    return 1;
}

/*
 * drains stuff in buffs 
 */
int
uart_drain(void)
{

    tcflush(comm_sock, TCIOFLUSH);
    tcdrain(comm_sock);

}

/*
 * Tries to sync packets by locking on to LOCAL_GROUP in the data stream 
 */
int
uart_sync(void)
{
    TOS_Msg         am_pkt;
    char           *byte_ptr = (char *) &am_pkt;
    int             i,
                    j;

    uart_drain();
    for (j = 0; j < 4; j++) {
	/*
	 * read upto an entire packet 
	 */
	if (recv_uart(&am_pkt) < 0) {
	    perror("recv msg_peek");
	    return -1;
	}

	/*
	 * See if the group is ours 
	 */
	printf("Sync ");
	print_packet(&am_pkt);
	if (am_pkt.group == LOCAL_GROUP) {
	    return 1;		/* Already synchronised */
	}

	/*
	 * Find the offset and read upto it 
	 */
	for (i = 2; i < AM_PKT_LEN; i++) {
	    if (byte_ptr[i] == LOCAL_GROUP) {
		printf("SYNC: %d\n", i - 2);
		/*
		 * read upto offset 
		 */
		if (read(comm_sock, byte_ptr, i - 2 * sizeof(char)) < 0)
		    return -1;

		/*
		 * synchronised 
		 */
		return 1;
	    }
	}
    }

    /*
     * failed 
     */
    return -1;
}

/*
 * Reads exactly one packet from the UART 
 */
int
recv_uart(TOS_Msg * msgPtr)
{
    int             recv_bytes;

    if (comm_sock < 0)
	if (uart_init() < 0)
	    return -1;
    while (1) {
	/*
	 * read full pkt 
	 */
	recv_bytes = read_all(comm_sock, msgPtr, AM_PKT_LEN);
	if (recv_bytes != AM_PKT_LEN)
	    return -1;
	/*
	 * return 
	 */
	return recv_bytes;
    }				/* End of while loop */
}

/*
 * Sends the given packet on the UART 
 */
int
send_uart(TOS_MsgPtr msgPtr)
{
    int             send_bytes;

    if (comm_sock < 0)
	if (uart_init() < 0)
	    return -1;

    // flow control - the lock must be unlockes when the FLOW_CONT is
    // received
    // this is done by another thread
    FLOW_LOCK;

    // send it
    send_bytes = write(comm_sock, msgPtr, AM_PKT_LEN);
    if (send_bytes < 0) {
	perror("While trying to write to UART");
	FLOW_UNLOCK;
	return -1;
    }
    // debug only
    tcdrain(comm_sock);
    usleep(COMM_SEND_DELAY);
    FLOW_UNLOCK;

    /*
     * return 
     */
    return send_bytes;
}
