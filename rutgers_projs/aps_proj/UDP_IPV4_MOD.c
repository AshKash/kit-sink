/*
 *
 * Copyright (c) 2001 Rutgers University and Richard P. Martin.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. All advertising materials mentioning features or use of this
 *       software must display the following acknowledgment:
 *           This product includes software developed by 
 *           Rutgers University and its contributors.
 *
 *    4. Neither the name of the University nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 * 
 * IN NO EVENT SHALL RUTGERS UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT, 
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF RUTGERS
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  
 * RUTGERS UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND RUTGERS UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *
 * Author:                      Richard P. Martin 
 * Version:                     $Id: UDP_IPV4_MOD.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:               Wed Aug 15 09:34:39 2001
 * Filename:                    UDP_IPV4_MOD.c
 * History:
 * $Log: UDP_IPV4_MOD.c,v $
 * Revision 1.2  2002/04/08 16:51:46  ashwink
 *
 * Dont panic!
 * Did a general cleanup to remove characters after #endif
 *
 * Revision 1.1.1.1  2002/03/08 23:52:37  ashwink
 *
 * APS Code includes:
 *      mote code
 *      mote-gw
 *      tcl/tk demo files
 *
 * Revision 1.5  2002/01/29 02:28:05  xili
 * working udp echo code
 *
 * Revision 1.4  2001/12/14 20:43:54  xili
 * working udp_echo module
 *
 * Revision 1.3  2001/12/08 22:56:26  xili
 * change for udp_echo
 *
 * Revision 1.2  2001/10/17 00:15:38  xili
 * work with UDP_ECHO
 *
 * Revision 1.1.1.1  2001/10/04 04:12:06  rmartin
 * hopefully this is correct
 *
 * Revision 1.3  2001/09/19 04:30:31  rmartin
 * Added byteswap routines
 *
 * Revision 1.2  2001/09/17 04:23:05  rmartin
 * Lots of fixes, but still broken.
 *
 * Revision 1.1  2001/09/06 04:21:46  rmartin
 * checking in so I don't lose them like the tos-ipv4.h
 *
 */


/*
 * This module implements a UDP 'socket' in TOS 
 */
/*
 * It not a socket, it's a TOS module. :-> 
 */

#include "tos.h"
#include "tos-ipv4.h"
#include "UDP_IPV4_MOD.h"

// #define FULLPC_DEBUG 1

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILED 0

#define CONFIG_PORT 0x2870	/* port to set an IP address */
#define UNIX_PORT 0x2880	/* unix box server port */

extern unsigned int udp_ipv4_checksum(char *data, int len);
char            process_tx(TOS_MsgPtr tx_msg, int len);
void            configure_IP_addr(ipv4hdr_t * ipv4_pkt);

// static char checkStack[11] = "galumbits!";


/*
 * ------------ Frame Declaration ---------- 
 */
#define TOS_FRAME_TYPE UDP_IPV4_MOD_frame
TOS_FRAME_BEGIN(UDP_IPV4_MOD_frame)
{
    char            state;	/* transmitting, receiving, escaping, or
				 * nothing? */
    TOS_MsgPtr      tx_ptr;	/* 'current' TX message pointer */
    TOS_MsgPtr      rx_ptr;	/* 'current' RX message pointer */
    ipv4addr_t      dest_addr;	/* 4 byte destination IPv4 address */
    unsigned int    remote_port;	/* destination UDP port number */
    unsigned int    local_port;	/* my UDP port number */
}
TOS_FRAME_END(UDP_IPV4_MOD_frame);


/*
 * ------------------ Initialization section --------------------
 */
char            TOS_COMMAND(UDP_IPV4_MOD_INIT) () {
    /*
     * this initialize is to help the IP address configuration 
     */
    VAR(dest_addr) = IPV4_BCAST_ADDR;
    VAR(remote_port) = UNIX_PORT;

    if (TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_INIT) () == HANDSHAKE_FAILED)
	return HANDSHAKE_FAILED;
    return HANDSHAKE_SUCCESS;
}

/*
 * no start code for now 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_START) () {
    return HANDSHAKE_SUCCESS;
}

char            TOS_COMMAND(UDP_IPV4_MOD_INIT_IP_ADDR) (TOS_MsgPtr tx_msg) {
    /*
     * almost the same as UDP_TX, just some part is hard-coded 
     */

    char           *data;
    int             len;

    /*
     * construct the data part 
     */

    // data = (char *)(tx_msg) + sizeof(ipv4hdr_t)+sizeof(udphdr_t);
    data = (char *) (tx_msg);
    data[0] = 0x62;
    data[1] = 0x62;
    len = 2;

    // XILIXILI STACK TEST
    // len = 48;


    if (process_tx(tx_msg, len)) {
	return HANDSHAKE_SUCCESS;
    }
    return HANDSHAKE_FAILED;
}

/*
 * most of the power commands return void. Does this violate the TOS
 * command-handshake protocol? 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_POWER) (char mode) {
    /*
     * no power-mode code for now, call UART power mode 
     */
    return (TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_POWER) (mode));
}

/*
 * ---------------- UDP configuration commands ---------------------
 */

/*
 * set the memory address of the IP address for this module 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_SET_DEST) (ipv4addr_t dest) {
    VAR(dest_addr) = dest;
    return HANDSHAKE_SUCCESS;
}

/*
 * set the local IP address for IPV4 module 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_SET_IP_ADDR) (ipv4addr_t addr) {
    return (TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_SET_IP_ADDR) (addr));
}

/*
 * get the memory address of the IP address for this module 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_GET_DEST) (ipv4addr_t * dest) {
    *dest = VAR(dest_addr);
    return HANDSHAKE_SUCCESS;
}

/*
 * set the remote port for this UDP module 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_SET_REMOTE_PORT) (unsigned int
							   remote) {
    VAR(remote_port) = remote;
    return HANDSHAKE_SUCCESS;
}

char            TOS_COMMAND(UDP_IPV4_MOD_GET_REMOTE_PORT) (unsigned int
							   *remote_p) {

    *remote_p = VAR(remote_port);
    return HANDSHAKE_SUCCESS;
}

/*
 * set the local port for this UDP module 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_SET_LOCAL_PORT) (unsigned int
							  local) {
    VAR(local_port) = local;
    return HANDSHAKE_SUCCESS;
}

char            TOS_COMMAND(UDP_IPV4_MOD_GET_LOCAL_PORT) (unsigned int
							  *local_p) {
    *local_p = VAR(local_port);
    return HANDSHAKE_SUCCESS;
}

/*
 * --------------------------Transmit side ------------------- 
 */
char            TOS_COMMAND(UDP_IPV4_MOD_TX_PACKET) (TOS_MsgPtr tx_msg,
						     int len) {
    return process_tx(tx_msg, len);
}

/*
 * get a TX done message from a lower layer. Not much here to do but pass 
 * the data up from a lower level environment 
 */
char            TOS_EVENT(UDP_IPV4_MOD_TX_DONE) (TOS_MsgPtr tx_msg) {
    /*
     * pass the event up the chain ... 
     */
    if (VAR(tx_ptr) == tx_msg) {
	VAR(state) = 0;
	TOS_SIGNAL_EVENT(UDP_IPV4_MOD_TX_PACKET_DONE) (tx_msg);
    }
    return EVENT_SUCCESS;
}

/*
 * -------------------------Receive side ------------------- 
 */

/*
 * this is the main RX routine. It's just a bunch of checks to make sure
 * the UDP packet is OK. If so, we signal the higher layer. If any check
 * failes, we bounce the buffer down to the lower layer without
 * signalling anything 
 */

// TOS_MsgPtr TOS_EVENT(UDP_IPV4_MOD_RX)(TOS_MsgPtr rx_msg, int len) { 
TOS_MsgPtr      TOS_EVENT(UDP_IPV4_MOD_RX) (TOS_MsgPtr rx_msg) {
    udphdr_t       *udphdr_p;
    ipv4hdr_t      *ipv4hdr_p;


#ifdef FULLPC_DEBUG
    char           *data;
    int             i;
    printf("UDP_IPV4_MOD_RX, got a message!\n");
#endif
    /*
     * get the UDP header 
     */
    /*
     * HELP! replace with the TOS-specific sizeof (HDR) call 
     */

    /*
     * so, we are assume no IP options are used??? 
     */

    udphdr_p = (udphdr_t *) ((char *) rx_msg +
			     sizeof(toshdr_t) + sizeof(ipv4hdr_t));
#ifdef FULLPC_DEBUG
    printf("udp header is:\n");
    for (i = 0; i < sizeof(udphdr_t); i++)
	printf("%x,", ((char *) udphdr_p)[i]);
    printf("\n");
    data = (char *) udphdr_p + sizeof(udphdr_t);
    printf("data is :\n");
    for (i = 0; i < 50; i++)
	printf("%x,", data[i]);
    printf("\n");
#endif

    /*
     * get the IP header 
     */
    /*
     * this hack is lame, but we must do it for now. Later will add the
     * header size chaining mumbo-jumbo 
     */
    ipv4hdr_p = (ipv4hdr_t *) ((char *) rx_msg + sizeof(toshdr_t));

    /*
     * verify the IP length is at least as long as the UDP length 
     */
    if ((ntohs(ipv4hdr_p->len)) <	/* total length of IP packet */
	((sizeof(ipv4hdr_t)) + ntohs(udphdr_p->len))) {	/* UDP + IP */
#ifdef FULLPC_DEBUG
	printf("drop the message, since it's too short!\n");
	printf
	    ("ipv4hdr_p->len = %d, sizeof(ipv4hdr_t) = %d, udphdr_p->len = %d\n",
	     ntohs(ipv4hdr_p->len), sizeof(ipv4hdr_t),
	     ntohs(udphdr_p->len));
#endif
	return (rx_msg);	/* Too short. dropped */
    }

    /*
     * verify the UDP checksum 
     */
    if (udphdr_p->check != 0) {
	/*
	 * reverse BSD tricks used in TX side 
	 */
	/*
	 * we are overiting IP header directly without saving it for later 
	 * use 
	 */
	ipv4hdr_p->ttl = 0;
	/*
	 * set the IP checksum to the UDP length (BSD trick) 
	 */
	ipv4hdr_p->check = udphdr_p->len;

	/*
	 * non-zero checksum is bad 
	 */
	if (udp_ipv4_checksum((char *) &(ipv4hdr_p->ttl), 3 * sizeof(uint32_t) +	/* pseudoheader */
			      ntohs(udphdr_p->len)) & 0xffff) {
#ifdef FULLPC_DEBUG
	    printf("drop the message, checksum failed!\n");
	    printf("the returned checksum is %x\n",
		   udp_ipv4_checksum(((char *) &ipv4hdr_p->ttl),
				     3 * sizeof(uint32_t) +
				     ntohs(udphdr_p->len)) & 0xffff);
#endif
	    return (rx_msg);	/* drop */
	}
    }


    /*
     * end if do a checksum 
     */
    /*
     * drop the other mote's configuration msg 
     */
    if (ipv4hdr_p->dest_addr == IPV4_BCAST_ADDR
	&& ntohs(udphdr_p->dst_port) == UNIX_PORT) {
	return (rx_msg);
    }

    /*
     * check if this is a IP ADDRESS configuration pkt! 
     */
    if (ipv4hdr_p->dest_addr == IPV4_BOOT_ADDR
	&& ntohs(udphdr_p->dst_port) == CONFIG_PORT) {
	configure_IP_addr(ipv4hdr_p);
	return (rx_msg);
    }
#ifdef FULLPC_DEBUG
    printf("UDP checking finished, go to echo!\n");
#endif

    /*
     * made it! pass up to next layer 
     */
    return (TOS_SIGNAL_EVENT(UDP_IPV4_MOD_RX_PACKET_DONE) (rx_msg));
}


/*
 * -------------------------Internal functions ------------------- 
 */

/*
 * compute the UDP checksum 
 */
/*
 * from RFC 1071 
 */

unsigned int
udp_ipv4_checksum(char *data, int len)
{

    uint32_t        sum;
    uint16_t        count;
    uint16_t       *addr;

    count = len;
    sum = 0;
    addr = (uint16_t *) data;

    while (count > 1) {
	/*
	 * This is the inner loop 
	 */
	sum += *(addr);

	addr++;			/* increment by 16 bits */
	count -= 2;		/* byte counter, subtract by 16 bits */
    }

    /*
     * Add left-over byte, if any 
     */
    if (count > 0)
	sum += *((unsigned char *) addr);

    /*
     * Fold 32-bit sum to 16 bits 
     */
    while (sum >> 16)
	sum = (sum & 0xffff) + (sum >> 16);

    return (~sum);
}


void
configure_IP_addr(ipv4hdr_t * ipv4_pkt)
{
    udphdr_t       *udp_pkt;
    ipv4addr_t     *addresses;

    udp_pkt = (udphdr_t *) ((char *) ipv4_pkt + sizeof(ipv4hdr_t));

    /*
     * check the length of the packet 
     */
    if (ntohs(udp_pkt->len) >= (2 * sizeof(ipv4addr_t))) {

	addresses = (ipv4addr_t *) ((char *) udp_pkt + sizeof(udphdr_t));

	/*
	 * set local ip & netmask
	 */
	TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_SET_IP_ADDR) (ntohl(addresses[0])
							|
							TOS_LOCAL_ADDRESS);
	TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_SET_NET_MASK) (ntohl
							 (addresses[1]));

	/*
	 * set the gateway address 
	 */

	TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_SET_GATEWAY) (ntohl
							(ipv4_pkt->
							 src_addr) &
							(~
							 (ntohl
							  (addresses
							   [1]))));

	TOS_CALL_COMMAND(UDP_ECHO_SUB_TOG_YELLOW) ();
    }
    return;
}



char
process_tx(TOS_MsgPtr tx_msg, int len)
{
    udphdr_t       *udphdr_p;
    ipv4hdr_t      *ipv4hdr_p;
    int             i,
                    range;

    if (!VAR(state)) {
#ifdef FULLPC_DEBUG
	printf
	    ("UDP_IPV4_MOD_TX_PACKET, the data I am going to send is:\n");
	for (i = 0; i < len; i++) {
	    printf("%x,", ((char *) tx_msg)[i]);
	}
	printf("\n");

#endif

	range = sizeof(toshdr_t) + sizeof(ipv4hdr_t) + sizeof(udphdr_t);

	/*
	 * data length shouldn't be longer than the buffer 
	 */
	if ((range + len + 1 - sizeof(toshdr_t)) > DATA_LENGTH)
	    return HANDSHAKE_FAILED;

	/*
	 * for(i=(len+range);i<sizeof(TOS_Msg);i++) ((char *)tx_msg)[i] =
	 * 0; 
	 */

	// TILL HERE STACK

	/*
	 * first, move the data to give enough space for udp,ip and tos
	 * header 
	 */

	for (i = len - 1; i >= 0; i--) {
	    ((char *) tx_msg)[i + range] = ((char *) tx_msg)[i];
	}

	/*
	 * hmm, there should be a better way to do this! 
	 */
	/*
	 * replace with the TOS-specific sizeof (HDR) call 
	 */
	udphdr_p = (udphdr_t *) ((char *) tx_msg +
				 sizeof(toshdr_t) + sizeof(ipv4hdr_t));

	/*
	 * this hack is lame, but we must do it for now. Later will add
	 * the header size chaining stuff 
	 */
	ipv4hdr_p = (ipv4hdr_t *) ((char *) tx_msg + sizeof(toshdr_t));

	/*
	 * set the fields in the UDP packet 
	 */
	udphdr_p->src_port = htons(VAR(local_port));

	udphdr_p->dst_port = htons(VAR(remote_port));
	udphdr_p->len = htons(len + sizeof(udphdr_t));

	/*
	 * recall the UDP pseudoheader is: |-------32 bit source IP addr
	 * ------| |-------32 bit dest IP addr --------| | zero
	 * |proto(17)| 16 bit UDP len-|
	 * 
	 * Then the header: | source port | dest port | | UDP length |
	 * checksum | 
	 */

	/*
	 * compute checksum 
	 */
	/*
	 * use the same trick as *bsd udp_output: steal fields from the
	 * IP header when computing the csum. IP will later overwrite
	 * these fields. It's nice the VAX was short of memory too. ;) 
	 */

	/*
	 * set the destination address in the IP layer 
	 */
	ipv4hdr_p->dest_addr = htonl(VAR(dest_addr));

	/*
	 * set the source address in the IP layer 
	 */
	TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_GET_SOURCE) (&ipv4hdr_p->
						       src_addr);
	ipv4hdr_p->src_addr = htonl(ipv4hdr_p->src_addr);

	/*
	 * set the protocol ID in the IP layer 
	 */
	ipv4hdr_p->protocol = UDP_PROTOCOL_ID;

	/*
	 * tricky. Reuse the field temporary for the csum pseudoheader 
	 */
	/*
	 * no need to byteswap here 
	 */
	ipv4hdr_p->check = udphdr_p->len;
	ipv4hdr_p->ttl = 0;

	udphdr_p->check = 0;
	udphdr_p->check = udp_ipv4_checksum((char *) &(ipv4hdr_p->ttl), 3 * sizeof(uint32_t) +	/* pseudoheader */
					    sizeof(udphdr_t) +	/* header */
					    len);

	/*
	 * record that this is the current TX message 
	 */

	ipv4hdr_p->ttl = MAX_TTL;
	VAR(tx_ptr) = tx_msg;

#ifdef FULLPC_DEBUG
	printf
	    ("UDP_IPV4_MOD_TX_PACKET, before calling IP module, the message is:\n");
	for (i = 0; i < range + len; i++) {
	    printf("%x,", ((char *) tx_msg)[i]);
	}
	printf("\n");
#endif

	/*
	 * send it to the lower layer 
	 */
	if (TOS_CALL_COMMAND(UDP_IPV4_MOD_SUB_TX_PACKET)
	    (tx_msg, len + range)) {
	    VAR(state) = 1;
	    return HANDSHAKE_SUCCESS;
	}
    }
    return HANDSHAKE_FAILED;
}
