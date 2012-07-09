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
 * Author:         Richard P. Martin 
 * Version:        $Id: SLIP_BRIDGE.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:  Wed Jul 18 21:41:36 2001
 * Filename:       SLIP_BRIDGE.c
 * History:
 * $Log: SLIP_BRIDGE.c,v $
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
 * Revision 1.4  2002/01/29 02:52:20  xili
 * working bridge side code with udp echo module
 *
 * Revision 1.3  2002/01/09 23:24:58  xili
 * temporary bridge side code for IP assignment
 *
 * Revision 1.2  2001/12/19 00:30:29  rmartin
 *
 * Modifications to automatically assign IP addresses to motes.
 * This code is still broken :(
 *
 * Revision 1.1.1.1  2001/10/04 04:12:06  rmartin
 * hopefully this is correct
 *
 * Revision 1.3  2001/08/10 15:00:14  rmartin
 * half-duplex bridge allows for larger packets
 *
 * Revision 1.2  2001/08/09 02:46:20  rmartin
 * Slip working in bi-directional mode; Able to flash by Node ID
 *
 * Revision 1.1  2001/08/06 20:22:16  rmartin
 * transmit from UART-> radio is working.
 *
 */

/*
 * A simple bridge(shunt) that connects the SLIP driver on the UART side
 * to the GENERIC_COMM component. Also connects the generic comm to the
 * SLIP driver. This was built as a test-harness for the SLIP driver.
 * Theory of operation: The driver is a classic buffer swapper type of
 * driver for an event-driven system (like TOS). Each low-level driver
 * (SLIP and Generic_comm), has a single RX buffer. The highest level
 * component, in this case the SLIP_BRIDGE, maintains a buffer for each
 * low-level driver. On an RX from either side, the SLIP_BRIDGE swaps one 
 * of its buffers for the driver's buffer. By having two buffers,
 * SLIP_BRIDGE allows for simulaneous full-duplex operation. Compare to
 * the half-duplex GENERIC_BASE. The component has a nice symmetrical
 * structure. The SLIP_BRIDGE modifies the first word of the message over 
 * the radio to include the length field from the SLIP driver. You have 
 * to compile with different destination addresses to make the bridge
 * work.
 * 
 */

#include "tos.h"
#include "tos-ipv4.h"

#ifdef FULLPC
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#define FULLPC_DEBUG 1
#include "SLIP_BRIDGE.h"

#define CONFIG_PORT 0x2870	/* port to get an IP address */
#define UNIX_PORT 0x2880	/* unix box server port */

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILIED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILIED 0

#define HALF_DUPLEX 1

/*
 * ------------ Frame Declaration ---------- 
 */
#ifdef FULL_DUPLEX
#define TOS_FRAME_TYPE SLIP_BRIDGE_frame
TOS_FRAME_BEGIN(SLIP_BRIDGE_frame)
{
    TOS_MsgPtr      radio_msg;	/* ptr to msg "owned" by the bridge for
				 * radio */
    TOS_MsgPtr      slip_msg;	/* ptr to msg "owned" by the bridge for
				 * slip slide */
    TOS_Msg         init_buffer1;	/* a base message buffer */
    TOS_Msg         init_buffer2;	/* a base message buffer */
    char            slip_tx_pending;	/* we have an outstanding TX
					 * message to the uart */
    char            radio_tx_pending;	/* outstanding message flag to the 
					 * radio */
    ipv4addr_t      subnet;	/* bit mask to get destination mote ID */
    ipv4addr_t      netmask;	/* bit mask to get destination mote ID */
}
TOS_FRAME_END(SLIP_BRIDGE_frame);
#endif

#ifdef HALF_DUPLEX
#define TOS_FRAME_TYPE SLIP_BRIDGE_frame
TOS_FRAME_BEGIN(SLIP_BRIDGE_frame)
{
    TOS_MsgPtr      radio_msg;	/* ptr to msg "owned" by the bridge for
				 * radio */
    TOS_Msg         init_buffer1;	/* a base message buffer */
    char            radio_tx_pending;	/* outstanding message flag to the 
					 * radio */
    ipv4addr_t      subnet;	/* bit mask to get destination mote ID */
    ipv4addr_t      netmask;	/* bit mask to get destination mote ID */
}
TOS_FRAME_END(SLIP_BRIDGE_frame);

#define slip_msg radio_msg
#define init_buffer2 init_buffer1
#define slip_tx_pending radio_tx_pending

#endif


void            set_network(ipv4hdr_t * ipv4_pkt);
char            get_network();
char            forward_setnetwork(TOS_MsgPtr forward_msg);
unsigned int    slip_checksum(char *data, int len);

/*
 * ------------------ Initialization section --------------------
 */
char            TOS_COMMAND(SLIP_BRIDGE_INIT) () {

    VAR(radio_msg) = (TOS_MsgPtr) & (VAR(init_buffer1));
    VAR(slip_msg) = (TOS_MsgPtr) & (VAR(init_buffer2));

    VAR(slip_tx_pending) = 0;
    VAR(radio_tx_pending) = 0;
#ifdef FULLPC_DEBUG
    printf("initialize of radio_tx_pending to %d ! (3)\n",
	   VAR(radio_tx_pending));
#endif

    /*
     * init the radio side then the slip side 
     */
    if (TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_RADIO_INIT) () ==
	HANDSHAKE_FAILIED) {
	return HANDSHAKE_FAILIED;
    }

    VAR(subnet) = 0x00000000;
    VAR(netmask) = 0x000000FF;

    TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_LED_INIT) ();

    // TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_RED)(); /* bang the red led */
    return (TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_SLIP_INIT) ());
}

/*
 * try to get an IP address and sub-net from the SLIP side 
 */
/*
 * send an IP broadcast packet on the SLIP side to port 
 */
char            TOS_COMMAND(SLIP_BRIDGE_START) () {

    printf("SLIP_BRIDGE, start, local id is : %d\n", TOS_LOCAL_ADDRESS);
    if (get_network()) {
	/*
	 * char *data = (char *) VAR(slip_msg); int i ;
	 * 
	 * for(i=0;i<4;i++){ data[i] = 'a'; }
	 * 
	 * if(TOS_CALL_COMMAND(SLIP_BRIDGE_TX_SLIP)(VAR(slip_msg),1)){ 
	 */
	return HANDSHAKE_SUCCESS;
    } else
	return HANDSHAKE_FAILIED;
}

/*
 * another power-mode command 
 */
char            TOS_COMMAND(SLIP_BRIDGE_POWER) (char mode) {

    TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_SLIP_POWER) (mode);
    TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_RADIO_POWER) (mode);

    return HANDSHAKE_SUCCESS;
}

/*
 * ------------------------ Radio side ---------------------------
 */

/*
 * handle an incomming message from the radio 
 */
TOS_MsgPtr      TOS_MSG_EVENT(SLIP_BRIDGE_RX_RADIO) (TOS_MsgPtr rx_msg) {
    TOS_MsgPtr      ret_msg;
    ipv4hdr_t      *ipv4hdr_p;	/* IP address header */
    udphdr_t       *udphdr_p;

    int             i,
                    len;

    // TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_RED)(); /* bang the green led
    // */

    ret_msg = rx_msg;		/* assume we're giving back the original
				 * buffer */

    if (VAR(slip_tx_pending) == 0) {
	// TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_GREEN)(); /* bang the
	// green led */

	/*
	 * swap the bridge SLIP TX buffer with the radio's RX buffer 
	 */
	ret_msg = VAR(slip_msg);
	VAR(slip_msg) = rx_msg;

	/*
	 * save the len field from the first word of the message 
	 */
	len = (int) (rx_msg->data[0]);

	/*
	 * and copy the rest of it in-place, thus nuking it 
	 */
	/*
	 * not efficient, but TOS msg's don't have a len field (yet) 
	 */
	for (i = 0; i < len; i++) {
	    rx_msg->data[i] = rx_msg->data[i + 1];
	}

	/*
	 * get the IP & UDP header 
	 */
	udphdr_p = (udphdr_t *) ((char *) rx_msg +
				 sizeof(toshdr_t) + sizeof(ipv4hdr_t));

	ipv4hdr_p = (ipv4hdr_t *) ((char *) rx_msg + sizeof(toshdr_t));

	// check the configure msg
	if (ipv4hdr_p->dest_addr == IPV4_BCAST_ADDR
	    && ntohs(udphdr_p->dst_port) != UNIX_PORT) {
	    return ret_msg;
	}

	/*
	 * we still need to forward the request reply to unix box 
	 */

#ifdef FULLPC_DEBUG
	printf
	    (" Radio side got a messge, about to forward to unix box !\n");
#endif

	/*
	 * call the slip side to TX the message 
	 */
	/*
	 * only forward the data part, not the AM header , but this
	 * already taken care by the slip side 
	 */
	if (TOS_CALL_COMMAND(SLIP_BRIDGE_TX_SLIP) (VAR(slip_msg), len)) {
	    // TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_RED)(); /* bang the
	    // green led */
	    VAR(slip_tx_pending) = 1;
	}
    }

    /*
     * give back some buffer to the radio (generic base) 
     */
    return ret_msg;
}

/*
 * This event signals the radio TX is done, which was caused by the the
 * SLIP RX event 
 */
char            TOS_EVENT(SLIP_BRIDGE_TX_RADIO_DONE) (TOS_MsgPtr tx_msg) {

    if (VAR(radio_msg) == tx_msg) {
	// TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_YELLOW)(); /* bang the
	// green led */
#ifdef FULLPC_DEBUG
	printf("change of radio_tx_pending from %d to 0! (3)\n",
	       VAR(radio_tx_pending));
#endif
	VAR(radio_tx_pending) = 0;
    }

    return EVENT_SUCCESS;
}

/*
 * ------------------------ SLIP side ----------------------------
 */
TOS_MsgPtr      TOS_EVENT(SLIP_BRIDGE_RX_SLIP) (TOS_MsgPtr rx_msg, int len) {

    TOS_MsgPtr      ret_msg;	/* message to send back to UART */
    int             i;
    ipv4hdr_t      *ipv4hdr_p;	/* IP address header */
    uint8_t         dest_mote;	/* destination mote */

#ifdef FULLPC
    printf("SLIP_BRIDGE, got a packet from the slip side!\n");
#endif


    ret_msg = rx_msg;		/* assume we're giving back the original
				 * buffer */

    if (VAR(radio_tx_pending) == 0) {

	/*
	 * swap the bridge radio TX buffer with the SLIP driver's RX
	 * buffer 
	 */
	ret_msg = VAR(radio_msg);
	VAR(radio_msg) = rx_msg;
	/*
	 * shift the bytes down by one. Should check the MTU here
	 */

	for (i = len; i > 0; i--) {
	    rx_msg->data[i] = rx_msg->data[i - 1];
	}

	/*
	 * insert the length into the 1st byte 
	 */
	rx_msg->data[0] = len;

	/*
	 * get the IP packet header 
	 */
	/*
	 * have to add 1 because the slip bridge inserts the length of the 
	 * * packet in front of the packet 
	 */
	ipv4hdr_p = (ipv4hdr_t *)
	    ((char *) VAR(radio_msg) + sizeof(toshdr_t) + 1);

	/*
	 * extract the destination address from the low-order bits of of
	 * the packet. 
	 */
	dest_mote = (uint8_t) (ntohl(ipv4hdr_p->dest_addr) & VAR(netmask));

#ifdef FULLPC
	printf("destination mote is %d\n", dest_mote);
#endif

	/*
	 * if the destination mote is zero, interperate this as a command 
	 * packet to the bridge 
	 */
	if (dest_mote != 0) {
	    /*
	     * call the radio side to TX the message 
	     */
	    if (TOS_CALL_COMMAND(SLIP_BRIDGE_TX_RADIO) (dest_mote,
							AM_MSG
							(SLIP_BRIDGE_RX_RADIO),
							VAR(radio_msg))) {
#ifdef FULLPC_DEBUG
		printf("change of radio_tx_pending from %d to 1! (1)\n",
		       VAR(radio_tx_pending));
#endif
		TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_GREEN) ();	/* bang
								 * the
								 * green
								 * led */
		VAR(radio_tx_pending) = 1;
	    }
	} else {
#ifdef FULLPC
	    printf("SLIP_BRIDGE, should call set_network now!\n");
#endif
	    set_network(ipv4hdr_p);
	    if (forward_setnetwork(VAR(radio_msg))) {
#ifdef FULLPC_DEBUG
		printf("change of radio_tx_pending from %d to 1! (2)\n",
		       VAR(radio_tx_pending));
#endif
		VAR(radio_tx_pending) = 1;
	    }
	}
    } else {
#ifdef FULLPC
	printf
	    ("SLIP_BRIDGE in radio tx mode, can't work on this right now!\n");
	printf("radio tx pending is : %d\n", VAR(radio_tx_pending));
#endif
    }

    /*
     * give back some buffer to the radio (generic base) 
     */
    return ret_msg;
}

/*
 * handle the event where the SLIP driver tells us it's done 
 */
/*
 * This even signals the SLIP/UART TX is done, which was caused by the
 * the Radio RX event 
 */
char            TOS_EVENT(SLIP_BRIDGE_TX_SLIP_DONE) (TOS_MsgPtr tx_msg) {

    if (VAR(slip_msg) == tx_msg) {
	VAR(slip_tx_pending) = 0;
    }

    return EVENT_SUCCESS;
}

/*
 * handle the event where the SLIP driver tells us it's done 
 */
/*
 * This even signals the SLIP/UART TX is done, which was caused by the
 * the Radio RX event 
 */
char            TOS_EVENT(SLIP_BRIDGE_RARP) (TOS_MsgPtr tx_msg) {

    if (VAR(slip_msg) == tx_msg) {
	VAR(slip_tx_pending) = 0;
    }

    return EVENT_SUCCESS;
}


/*
 * -------------------------Internal functions ------------------- 
 */
void
set_network(ipv4hdr_t * ipv4_pkt)
{
    udphdr_t       *udp_pkt;
    ipv4addr_t     *addresses;

#ifdef FULLPC
    struct in_addr  addr;
    char            names[30];
    char           *addrs = names;
    printf("this is a configure packet!\n");
    addr.s_addr = htonl(VAR(netmask));
    addrs = inet_ntoa(addr);
    printf("before configuration, netmask is:%s!\n", addrs);
    addr.s_addr = htonl(VAR(subnet));
    addrs = inet_ntoa(addr);
    printf("before configuration, subnet is:%s!\n", addrs);
#endif

    udp_pkt = (udphdr_t *) ((char *) ipv4_pkt + sizeof(ipv4hdr_t));


    /*
     * check the port and the length of the packet 
     */
    if ((ntohs(udp_pkt->dst_port) == CONFIG_PORT) &&
	(ntohs(udp_pkt->len) >= (2 * sizeof(ipv4addr_t)))) {

	addresses = (ipv4addr_t *) ((char *) udp_pkt + sizeof(udphdr_t));
	VAR(subnet) = ntohl(addresses[0]);
	// VAR(netmask) = ntohl(addresses[1]); 
	VAR(netmask) = ~(ntohl(addresses[1]));

	// get_network();


#ifdef FULLPC
	addr.s_addr = htonl(VAR(netmask));
	addrs = inet_ntoa(addr);
	printf("after configuration, netmask is:%s!\n", addrs);
	addr.s_addr = htonl(VAR(subnet));
	addrs = inet_ntoa(addr);
	printf("after configuration, subnet is:%s!\n", addrs);
#endif

	return;
    }
#ifdef FULLPC
    printf("Configure packet error!\n");
#endif

    return;
}

char
forward_setnetwork(TOS_MsgPtr forward_msg)
{
    ipv4hdr_t      *ipv4_pkt;
    udphdr_t       *udp_pkt;

    ipv4_pkt = (ipv4hdr_t *)
	((char *) forward_msg + sizeof(toshdr_t) + 1);

    udp_pkt = (udphdr_t *) ((char *) ipv4_pkt + sizeof(ipv4hdr_t));

    /*
     * change the destination address, recompute checksum 
     */
    ipv4_pkt->dest_addr = IPV4_BOOT_ADDR;	/* all 0's */
    ipv4_pkt->check = udp_pkt->len;
    ipv4_pkt->ttl = 0;		/* no need to remember the original value, 
				 * it doesn't have * too much meaning in
				 * mote communication */
    udp_pkt->check = 0;
    udp_pkt->check = slip_checksum((char *) &(ipv4_pkt->ttl), 3 * sizeof(uint32_t) +	/* pseudoheader */
				   ntohs(udp_pkt->len));
    ipv4_pkt->ttl = MAX_TTL;
    ipv4_pkt->check = 0;
    ipv4_pkt->check = slip_checksum((char *) ipv4_pkt, sizeof(ipv4hdr_t));

    if (TOS_CALL_COMMAND(SLIP_BRIDGE_TX_RADIO) (TOS_BCAST_ADDR,
						AM_MSG
						(SLIP_BRIDGE_RX_RADIO),
						forward_msg)) {
	TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_YELLOW) ();	/* bang
								 * the red 
								 * led */
	return HANDSHAKE_SUCCESS;
    } else
	return HANDSHAKE_FAILIED;

}

char
get_network()
{
    ipv4hdr_t      *ipv4hdr_p;	/* IP address header */
    udphdr_t       *udp_pkt;
    char           *data;
    // ,*msg;
    // ipv4addr_t *addresses; 
    // int i;

    // construct the query message 

    ipv4hdr_p = (ipv4hdr_t *)
	((char *) VAR(slip_msg) + sizeof(toshdr_t));
    udp_pkt =
	(udphdr_t *) ((char *) VAR(slip_msg) + sizeof(toshdr_t) +
		      sizeof(ipv4hdr_t));
    data = (char *) ((char *) udp_pkt + sizeof(udphdr_t));

    data[0] = 0x61;
    data[1] = 0x61;
    /*
     * data[2] = 0x61; data[3] = 0x61; data[4] = 0x61; data[5] = 0x61;
     * data[6] = 0x61; data[7] = 0x61; 
     */

    // VAR(subnet) = htonl(VAR(subnet));
    // VAR(netmask) = htonl(VAR(netmask));
    // memcpy((void *)data,(void *)&(VAR(subnet)),4);
    // strncpy((char *)data,(char *)&(VAR(subnet)),4);
    // (char *)data = (char *)data+4;
    // memcpy((void *)data,(void *)&(VAR(netmask)),4);
    // strncpy((void *)data,(char *)&(VAR(netmask)),4);

    udp_pkt->src_port = htons(CONFIG_PORT);
    udp_pkt->dst_port = htons(UNIX_PORT);
    udp_pkt->len = htons(0xA);	// 8bytes header + 2 bytes payload

    ipv4hdr_p->src_addr = 0;
    ipv4hdr_p->dest_addr = 0xFFFFFFFF;
    ipv4hdr_p->protocol = 0x11;
    ipv4hdr_p->check = udp_pkt->len;
    ipv4hdr_p->ttl = 0x0;

    udp_pkt->check = 0;
    udp_pkt->check = slip_checksum((char *) &(ipv4hdr_p->ttl),
				   3 * sizeof(uint32_t) +
				   sizeof(udphdr_t) + 2);
    // 2 is the data length

    ipv4hdr_p->hl_version = 0x45;
    ipv4hdr_p->tos = 0x0;
    ipv4hdr_p->len = htons(0x001E);	// 28 bytes + 2byte
    // ipv4hdr_p -> len = htons(0x0024); //28 bytes + 8byte
    ipv4hdr_p->id = 0x0;
    ipv4hdr_p->off = 0x0;
    // ipv4hdr_p -> ttl = 0x1; //only need to go one hop away to the base
    // station
    ipv4hdr_p->ttl = 0x3;	// only need to go one hop away to the
				// base station
    ipv4hdr_p->check = 0x0;
    ipv4hdr_p->check =
	slip_checksum((char *) ipv4hdr_p, sizeof(ipv4hdr_t));

    printf("get_network, will send to uart a packet with length: %d\n",
	   (int) (ntohs(ipv4hdr_p->len)));
    /*
     * call the slip side to TX the message 
     */
    if (TOS_CALL_COMMAND(SLIP_BRIDGE_TX_SLIP)
	(VAR(slip_msg), (int) (ntohs(ipv4hdr_p->len)))) {
	printf("sent the get_network request to the base station \n");
	VAR(slip_tx_pending) = 1;
	// TOS_CALL_COMMAND(SLIP_BRIDGE_SUB_TOG_GREEN)(); /* bang the red
	// led */
	return HANDSHAKE_SUCCESS;
    } else
	return HANDSHAKE_FAILIED;
}


unsigned int
slip_checksum(char *data, int len)
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
