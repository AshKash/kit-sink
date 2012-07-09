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
 * Version:                     $Id: IPV4_SIMPLE.c,v 1.2 2002/04/08 16:51:44 ashwink Exp $
 * Creation Date:               Fri Aug 10 11:47:16 2001
 * Filename:                    IPV4_SIMPLE.c
 * History:
 * $Log: IPV4_SIMPLE.c,v $
 * Revision 1.2  2002/04/08 16:51:44  ashwink
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
 * Revision 1.8  2002/01/29 02:28:05  xili
 * working udp echo code
 *
 * Revision 1.7  2002/01/09 23:49:40  xili
 * temporary working code for IP assignment
 *
 * Revision 1.6  2001/12/19 00:30:28  rmartin
 *
 * Modifications to automatically assign IP addresses to motes.
 * This code is still broken :(
 *
 * Revision 1.5  2001/12/14 20:43:54  xili
 * working udp_echo module
 *
 * Revision 1.4  2001/12/08 22:56:26  xili
 * change for udp_echo
 *
 * Revision 1.3  2001/10/17 00:15:13  xili
 * work with UDP_ECHO
 *
 * Revision 1.2  2001/10/04 04:53:08  rmartin
 * added UDP echo component
 *
 * Revision 1.3  2001/09/19 04:30:31  rmartin
 * Added byteswap routines
 *
 * Revision 1.2  2001/09/17 04:23:04  rmartin
 * Lots of fixes, but still broken.
 *
 * Revision 1.1  2001/09/06 04:21:45  rmartin
 * checking in so I don't lose them like the tos-ipv4.h
 *
 */

#include "tos.h"
#include "tos-ipv4.h"
#include "IPV4_SIMPLE.h"

// #define FULLPC_DEBUG 1

#ifdef FULLPC
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define UNIQ_START  0x0328

/*
 * simple routing table has 2 routes 
 */
#define MAX_IPV4_ROUTES 2
#define MAX_IPV4_DEMUX 2

#define TOS_FRAME_TYPE IPV4_SIMPLE_frame
TOS_FRAME_BEGIN(IPV4_SIMPLE_frame)
{
    char            state;
    TOS_MsgPtr      tx_ptr;	/* transmit message pointer */
    TOS_MsgPtr      rx_ptr;	/* receive message pointer */
    ipv4addr_t      my_ipv4;	/* my local address */
    ipv4addr_t      netmask;	/* my netmask for this network */
    char            gateway;	/* gateway mote ID */
    uint16_t        uniq_id;	/* unique ID per packet */
    char            demux_tab[MAX_IPV4_DEMUX];	/* how many higher layers
						 * do we support? */
}
TOS_FRAME_END(IPV4_SIMPLE_frame);

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILED 0
#define NULL         0

#define IPVERSION 4

unsigned int    ipv4_checksum(char *, int);


/*
 * ------------------ Initialization section --------------------
 */
char            TOS_COMMAND(IPV4_SIMPLE_INIT) () {
    VAR(uniq_id) = UNIQ_START;
    VAR(state) = 0;
    VAR(my_ipv4) = IPV4_BOOT_ADDR;
    VAR(netmask) = 0x00000000;
    if (TOS_CALL_COMMAND(IPV4_SIMPLE_SUB_INIT) () == HANDSHAKE_FAILED)
	return HANDSHAKE_FAILED;
    return HANDSHAKE_SUCCESS;
}

/*
 * no start code for now 
 */
char            TOS_COMMAND(IPV4_SIMPLE_START) () {
    return HANDSHAKE_SUCCESS;
}

/*
 * most of the power commands return void. Does this violate the TOS
 * command-handshake protocol? 
 */
char            TOS_COMMAND(IPV4_SIMPLE_POWER) (char mode) {
    /*
     * no power-mode code for now, call UART power mode 
     */
    return (TOS_CALL_COMMAND(IPV4_SIMPLE_SUB_POWER) (mode));
}

/*
 * ---------------- IPV4 configuration commands ---------------------
 */

/*
 * set the IP address of this module 
 */
char            TOS_COMMAND(IPV4_SIMPLE_SET_LOCAL_ADDR) (ipv4addr_t addr) {

#ifdef FULLPC_DEBUG
    struct in_addr  addrS;
    char            names[30];
    char           *addrs = names;
    addrS.s_addr = htonl(VAR(my_ipv4));
    addrs = inet_ntoa(addrS);
    printf("before configuration, local ip is:%s!\n", addrs);
#endif

    VAR(my_ipv4) = addr;

#ifdef FULLPC_DEBUG
    addrS.s_addr = htonl(VAR(my_ipv4));
    addrs = inet_ntoa(addrS);
    printf("after configuration, local ip is:%s!\n", addrs);
#endif

    return HANDSHAKE_SUCCESS;
}

/*
 * set IP address of this module 
 */
char            TOS_COMMAND(IPV4_SIMPLE_GET_LOCAL_ADDR) (ipv4addr_t *
							 local_p) {

    *local_p = VAR(my_ipv4);

    return HANDSHAKE_SUCCESS;
}

/*
 * set the demux table that maps the protocol ID to higher-level
 * component 
 */

char            TOS_COMMAND(IPV4_SIMPLE_SET_DEMUX) (int entry,
						    uint8_t protoID) {

    return HANDSHAKE_SUCCESS;
}

/*
 * set the netmask 
 */
char            TOS_COMMAND(IPV4_SIMPLE_SET_NET_MASK) (ipv4addr_t mask) {

#ifdef FULLPC_DEBUG
    struct in_addr  addr;
    char            names[30];
    char           *addrs = names;
    addr.s_addr = htonl(VAR(netmask));
    addrs = inet_ntoa(addr);
    printf("before configuration, net mask is:%s!\n", addrs);
#endif

    VAR(netmask) = mask;

#ifdef FULLPC_DEBUG
    addr.s_addr = htonl(VAR(netmask));
    addrs = inet_ntoa(addr);
    printf("after configuration, netmask is:%s!\n", addrs);
#endif
    return HANDSHAKE_SUCCESS;
}

/*
 * set gateway id 
 */
char            TOS_COMMAND(IPV4_SIMPLE_SET_GATEWAY) (char id) {

    VAR(gateway) = id;
    return HANDSHAKE_SUCCESS;
}


/*
 * --------------------------Transmit side ------------------- 
 */
char            TOS_COMMAND(IPV4_SIMPLE_TX_PACKET) (TOS_MsgPtr tx_msg,
						    int len) {
    ipv4hdr_t      *ipv4hdr_p;
    char            dest_mote;
#ifdef FULLPC_DEBUG
    char           *data;
#endif
    int             i;
#ifdef CHECK_STACK
    void           *addr1,
                   *addr2,
                   *addr3;
    char           *test;

    addr1 = &tx_msg;
    addr2 = &len;
    addr3 = &ipv4hdr_p;
#endif

    if (VAR(state) == 0) {
	/*
	 * Pleaze add the header size chaining here 
	 */
	ipv4hdr_p = (ipv4hdr_t *) ((char *) tx_msg + sizeof(toshdr_t));

	/*
	 * set the version number 
	 */
	ipv4hdr_p->hl_version =
	    (((IPVERSION << 4) & 0xff) |
	     ((sizeof(ipv4hdr_t) >> 2) & 0xff));
	// ipv4hdr_p->hl_version = 4<<4;

	/*
	 * no options or reassambly 
	 */
	/*
	 * set the fields in the IP packet 
	 */
	ipv4hdr_p->tos = 0;
	ipv4hdr_p->len = htons(len - sizeof(toshdr_t));
	ipv4hdr_p->id = VAR(uniq_id);
	VAR(uniq_id) = VAR(uniq_id)++;

	ipv4hdr_p->off = 0;

	/*
	 * don't set the destination address in the IP layer, assume 
	 */
	/*
	 * a higher layer like TCP or UDP has done it, unless it's zero 
	 */

	/*
	 * compute header checksum 
	 */
	ipv4hdr_p->ttl = MAX_TTL;
	ipv4hdr_p->protocol = UDP_PROTOCOL_ID;
	ipv4hdr_p->check = 0;

	ipv4hdr_p->check =
	    ipv4_checksum((char *) ipv4hdr_p, sizeof(ipv4hdr_t));

	/*
	 * record that this is the current TX message 
	 */
	VAR(tx_ptr) = tx_msg;

#ifdef FULLPC_DEBUG
	printf("IPV4_SIMPLE_TX, before call AM, the message is:\n");
	for (i = 0; i < len; i++) {
	    printf("%x,", ((char *) tx_msg)[i]);
	}
	printf("\n");
#endif

	/*
	 * send it to the lower layer 
	 */

	// get the destination address before move the data
	if ((ntohl(ipv4hdr_p->dest_addr) & VAR(netmask)) ==
	    (VAR(my_ipv4) & VAR(netmask))) {
	    /*
	     * it is applicable to the configuration case also 
	     */
	    dest_mote = (ntohl(ipv4hdr_p->dest_addr) & (~(VAR(netmask))));
	} else {
	    /*
	     * if the destination is not on the same subnet, use the
	     * gateway mote 
	     */

	    dest_mote = VAR(gateway);
	}

	for (i = len; i > 0; i--) {
	    tx_msg->data[i] = tx_msg->data[i - 1];
	}
	tx_msg->data[0] = (len - sizeof(toshdr_t));


	if (TOS_CALL_COMMAND(IPV4_SIMPLE_SUB_TX_PACKET)
	    (dest_mote, AM_MSG(IPV4_SIMPLE_RX), tx_msg)) {
	    VAR(state) = 1;
	    return HANDSHAKE_SUCCESS;
	}
    }
    return HANDSHAKE_FAILED;
}

/*
 * get a TX done message from a lower layer. Not much here to do but pass 
 * the data up from a lower level environment 
 */
char            TOS_EVENT(IPV4_SIMPLE_TX_DONE) (TOS_MsgPtr tx_msg) {

    /*
     * pass the event up the chain ... 
     */
    if (VAR(tx_ptr) == tx_msg) {
	VAR(state) = 0;
	TOS_SIGNAL_EVENT(IPV4_SIMPLE_TX_PACKET_DONE) (tx_msg);
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

TOS_MsgPtr      TOS_EVENT(IPV4_SIMPLE_RX) (TOS_MsgPtr rx_msg) {
    ipv4hdr_t      *ipv4hdr_p;
    int             mlen,
                    i;
    int             version,
                    hlen;
#ifdef FULLPC_DEBUG
    printf("IPV4_SIMPLE_RX, got a message!\n");
#endif


    // This is due to the sending problem of SLIP_BRIDGE, should get rid
    // of it later
    /*
     * get rid of the length part from AM message 
     */
    mlen = (int) (rx_msg->data[0]);

    /*
     * the length should not be less than IP header length 
     */
    if (mlen < sizeof(ipv4hdr_t)) {
	return (rx_msg);
    }

    /*
     * and copy the rest of it in-place, thus nuking it 
     */
    for (i = 0; i < mlen; i++) {
	rx_msg->data[i] = rx_msg->data[i + 1];
    }


#ifdef FULLPC_DEBUG
    // printf("message length is %d\n",mlen);
    printf("my_ipv4 is :%u or %x\n", VAR(my_ipv4), VAR(my_ipv4));
#endif

    /*
     * get the IP header 
     */
    ipv4hdr_p = (ipv4hdr_t *) ((char *) rx_msg + sizeof(toshdr_t));
#ifdef FULLPC_DEBUG
    printf("IP header is:\n");
    for (i = 0; i < sizeof(ipv4hdr_t); i++)
	printf("%x,", ((char *) ipv4hdr_p)[i]);
    printf("\n");
#endif

    /*
     * check the version 
     */
    version = ipv4hdr_p->hl_version >> 4;
    if (version != IPVERSION) {
#ifdef FULLPC_DEBUG
	printf("version inconsistency, drop the ip packet!\n");
#endif
	return (rx_msg);
    }

    /*
     * check the header length 
     */
    hlen = (ipv4hdr_p->hl_version & 0x0f) << 2;
    if (hlen < sizeof(ipv4hdr_t)) {
#ifdef FULLPC_DEBUG
	printf("too short, drop the ip packet!\n");
#endif
	return (rx_msg);
    }

    /*
     * verify the header checksum 
     */
    if (ipv4_checksum((char *) ipv4hdr_p, hlen) & 0xffff) {
#ifdef FULLPC_DEBUG
	printf("checksum fail, drop the ip packet!\n");
	printf("the returned check sum is %x or %d\n",
	       ipv4_checksum((char *) ipv4hdr_p, hlen),
	       ipv4_checksum((char *) ipv4hdr_p, hlen));
#endif
	return (rx_msg);
    }

    /*
     * total packet length should be longer than header length 
     */
    if ((ntohs(ipv4hdr_p->len)) < hlen) {
	return (rx_msg);
    }

    /*
     * is this packet to me or broadcast? 
     */

    if ((ntohl(ipv4hdr_p->dest_addr) != VAR(my_ipv4))
	&& (ntohl(ipv4hdr_p->dest_addr) != IPV4_BCAST_ADDR)
	&& (ntohl(ipv4hdr_p->dest_addr) != IPV4_BOOT_ADDR)
	&& (ntohl(ipv4hdr_p->dest_addr) !=
	    (VAR(my_ipv4) | ~(VAR(netmask))))) {

#ifdef FULLPC_DEBUG
	printf("the destination is not me, drop the ip packet!\n");
	printf("addr in the packet is : %x\n",
	       ntohl(ipv4hdr_p->dest_addr));
#endif
	return (rx_msg);
    }

    /*
     * should add the demultiplex here !!!
     */
    /*
     * made it! pass up to next layer 
     */
    return (TOS_SIGNAL_EVENT(IPV4_SIMPLE_RX_PACKET_DONE) (rx_msg));
}


/*
 * -------------------------Internal functions ------------------- 
 */

/*
 * compute the IP checksum 
 */
/*
 * from RFC 1071 
 */

unsigned int
ipv4_checksum(char *data, int len)
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
	sum += *((uint16_t *) addr);

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
