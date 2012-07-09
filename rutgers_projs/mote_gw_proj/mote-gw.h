#ifndef MOTE_PROXY

/* Misc includes */
#include <pthread.h>
#include "MSG.h"

#define MOTE_PROXY
#define AM_CLIENT_REG (0xFF)
#define AM_TYPE_REG   (0xFE)
#define TABLE_SIZE 256

/* This is needed for flow control, must be same as in MOTE-GW.c */
#define FLOW_STOP ((char) 0xe)
#define FLOW_CONT ((char) 0xf)

/* Error codes */
enum {
  E_UNKNOWN,       /* Unknown error */
  E_TOUT,          /* Timeout */
  E_NOGRP,         /* Invalid GID */
  E_NOMEM = 0x4,    /* No memory */
  E_PROT = 0x8,     /* Invalid protocol */
  E_SERIAL = 0x10   /* Serial transmission error */
} err_codes;

/* ################### High Level Functions ##################### */


/* ################### Low Level Functions ##################### */

/*
 * connects to a server and returns the socket
 * takes the host name and the port
 */
int connect_to_server (const char *hostname, int port_number);


/* Client initialization routine. Must be called by all clients before 
 * starting communications.
 * The server_socket will be returned, which the client can make use
 * for sending and receiving queries. The function can only be called
 * once during client startup */
int client_init( int argc, char **argv);

/* Reads upto buff_size or dies on error. Returns number of bytes read */
int read_or_die(int socket, void *buff, size_t buff_size);

/* Keeps reading until the needed number of bytes are received
   returns athe actual number of bytes read  */
int read_all(int socket, void *buff, size_t buff_size);

/* Keeps reading until the needed number bytes are received */
int read_all_or_die(int socket, void *buff, size_t buff_size);

/* Writes given number of bytes or dies on error */
int write_or_die(int socket, void *buff, size_t buff_size);

/* Sends the given TOS msg over the socket. Returns -1 on failure */
int send_tos_msg(int socket, TOS_Msg *tos_msg_ptr);

/* Receives a TOS msg over the socket and stores it */
int recv_tos_msg(int socket, TOS_Msg *tos_msg_ptr);

/* Registers the am type with a handler. Whenever a AM is received with the
 * specified type, the registered handler will be called
 * All other types of AMs will be dropped
 * The function must return the processed packet if it needs to be 
 * transmitted back, or a NULL to drop the packet */
int reg_am_type(char type, TOS_Msg * am_handler(TOS_Msg *am_pkt));

/* Deregister the handler */
int dereg_am_type(char type);

/* Reads on a socket and receives all TOS_Msg packets. It then calls the
 * appropriate handler for the type */
void * type_listener(void *arg);

/* Client Usage */
void usage(void);

#endif
