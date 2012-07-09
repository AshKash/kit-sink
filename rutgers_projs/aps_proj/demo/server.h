#include "mote-gw.h"

/*
 ******** Function prototypes ***********
 */

/* The thread to be called after accept() */
void *do_chld(void *arg);

/*
 * This function binds to specified port and creates a new
 * thread when accept() returns
 * The socket accept() returns (not the ptr) is passed to the
 * function.
 * Never returns unless there is an error or server is killed.
 */
void bind_accept_fork(int bind_port, void* (*start_thread) (void *));

/* This function is called when the type of the AM is AM_CLIENT_REG
 * It will associate the address contained within the AM with the socket
 * that the message was received on */
int am_client_reg(int socket);

/* Deregister a client */
void am_client_dereg(int socket);

/* Reverse lookup the addr_table */
int get_mote_id(int socket);

/* UART stuff */

/* This function will read a AM packet from the serial and send the 
 * packet to all appropriate clients
 * Locks and unlocks the addr_table as needed */
void * uart_listener(void *args);


/* Initialize the UART */
int uart_init(void);

/* Reads exactly one packet from the UART */
int recv_uart(TOS_Msg *msgPtr);

/* Sends the given packet on the UART */
int send_uart( TOS_MsgPtr msgPtr);
