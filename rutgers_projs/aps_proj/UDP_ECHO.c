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
 * Version:                     $Id: UDP_ECHO.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:               Thu Sep  6 23:25:23 2001
 * Filename:                    UDP_ECHO.c
 * History:
 * $Log: UDP_ECHO.c,v $
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
 * Revision 1.7  2002/01/29 02:28:05  xili
 * working udp echo code
 *
 * Revision 1.6  2002/01/09 23:49:40  xili
 * temporary working code for IP assignment
 *
 * Revision 1.5  2001/12/19 00:30:29  rmartin
 *
 * Modifications to automatically assign IP addresses to motes.
 * This code is still broken :(
 *
 * Revision 1.4  2001/12/14 20:43:54  xili
 * working udp_echo module
 *
 * Revision 1.3  2001/12/08 22:56:27  xili
 * change for udp_echo
 *
 * Revision 1.2  2001/10/17 00:16:04  xili
 * work with UDP_ECHO
 *
 * Revision 1.1  2001/10/04 04:53:09  rmartin
 * added UDP echo component
 *
 */

#include "tos.h"
#include "tos-ipv4.h"

// #define FULLPC_DEBUG 1
#include "UDP_ECHO.h"

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILED 0

#define UDP_ECHO_PORT  1101

/*
 * ------------ Frame Declaration ---------- 
 */
#define TOS_FRAME_TYPE UDP_ECHO_frame
TOS_FRAME_BEGIN(UDP_ECHO_frame)
{
    TOS_MsgPtr      current_msg;	/* ptr msg owned by US */
    TOS_Msg         init_buffer1;	/* a base message buffer */
    char            tx_pending;	/* we have an outstanding TX message to
				 * the uart */
}
TOS_FRAME_END(UDP_ECHO_frame);

// static char my_name[31]="abcdefghijklmnopqrstuvwxyzabc";
// char __attribute__((progmem))
// my_name[49]="galumbits,galumbits,galumbits,galumbits,galumbit";
// static char
// my_name[49]="galumbits,galumbits,galumbits,galumbits,galumbit";

/*
 * ------------------ Initialization section --------------------
 */
char            TOS_COMMAND(UDP_ECHO_INIT) () {
    printf("starting UDP echo test\n");

    VAR(current_msg) = (TOS_MsgPtr) & VAR(init_buffer1);
    VAR(tx_pending) = 0;

    /*
     * init the radio side then the slip side 
     */
    if (TOS_CALL_COMMAND(UDP_ECHO_SUB_INIT) () == HANDSHAKE_FAILED) {
	return HANDSHAKE_FAILED;
    }

    TOS_CALL_COMMAND(UDP_ECHO_SUB_LED_INIT) ();

    /*
     * set the local UDP port number 
     */
    TOS_CALL_COMMAND(UDP_ECHO_SUB_SET_UDP_PORT) ((uint16_t) UDP_ECHO_PORT);

    return HANDSHAKE_SUCCESS;
}

char            TOS_COMMAND(UDP_ECHO_START) () {

    /*
     * try to get an IP address and netmask 
     */
    if (TOS_CALL_COMMAND(UDP_ECHO_SUB_INIT_IP_ADDR) (VAR(current_msg))) {
	VAR(tx_pending) = 1;
	return HANDSHAKE_SUCCESS;
    } else
	return HANDSHAKE_FAILED;
}

/*
 * another power-mode command 
 */
char            TOS_COMMAND(UDP_ECHO_POWER) (char mode) {

    TOS_CALL_COMMAND(UDP_ECHO_SUB_POWER) (mode);

    return HANDSHAKE_SUCCESS;
}

/*
 * handle an incomming message 
 */
TOS_MsgPtr      TOS_MSG_EVENT(UDP_ECHO_RX) (TOS_MsgPtr rx_msg) {

    TOS_MsgPtr      ret_msg;
    ipv4hdr_t      *ipp =
	(ipv4hdr_t *) ((char *) rx_msg + sizeof(toshdr_t));
    udphdr_t       *udpp =
	(udphdr_t *) ((char *) rx_msg + sizeof(toshdr_t) +
		      sizeof(ipv4hdr_t));
    char           *data;
    int             i,
                    dataLen;

    /*
     * void *addr1,*addr2,*addr3; addr1 = &rx_msg; addr2 = &ret_msg; addr3 
     * = &ipp; 
     */

    TOS_CALL_COMMAND(UDP_ECHO_SUB_TOG_GREEN) ();	/* bang the green
							 * led */

    ret_msg = rx_msg;		/* assume we're giving back the original
				 * buffer */

    if (VAR(tx_pending) == 0) {
#ifdef FULLPC_DEBUG
	printf("UDP_ECHO_RX, got a message, actual data is:\n");
	for (i = 0; i < (ntohs(udpp->len) - sizeof(udphdr_t)); i++) {
	    printf("%x,", ((char *) udpp + sizeof(udphdr_t))[i]);
	}
	printf("\n");
#endif

	// buffer swap
	ret_msg = VAR(current_msg);
	VAR(current_msg) = rx_msg;

#ifdef FULLPC_DEBUG
	data = (char *) &(udpp->dst_port);
	printf("corresponding dest port is: %x,%x\n", data[0], data[1]);
#endif

	/*
	 * check if this is a msg for udp_echo 
	 */
	if ((ntohs(udpp->dst_port) != UDP_ECHO_PORT)) {
	    return ret_msg;
	} else {
#ifdef FULLPC_DEBUG
	    printf("this is a normal msg for udp echo\n");
#endif

	    // preparation to echo back to the sender
	    TOS_CALL_COMMAND(UDP_ECHO_SUB_SET_DEST) (ntohl(ipp->src_addr));
	    TOS_CALL_COMMAND(UDP_ECHO_SUB_SET_UDP_REMOTE_PORT) (ntohs
								(udpp->
								 src_port));

	    data = (char *) udpp + sizeof(udphdr_t);
	    // ECHO DATA

	    dataLen = (ntohs(udpp->len) - sizeof(udphdr_t));
	    for (i = 0; i < dataLen; i++) {
		*(((char *) VAR(current_msg)) + i) = *(data + i);
	    }


	    // TEST THE STACK XILI
	    /*
	     * //data = VAR(current_msg); //data = (char *)udpp +
	     * sizeof(udphdr_t); memcpy((void *)data,(void *)&addr1,2);
	     * memcpy((void *)(data+2),(void *)&addr2,2); memcpy((void
	     * *)(data+4),(void *)&addr3,2); 
	     */

	    /*
	     * 
	     * data[0] = '1'; data[1] = '$'; data[2] = '<'; data[3] = '^';
	     * data[4] = 'B'; data[5] = 'a'; data[6] = 'd'; data[7] = 'r';
	     * data[8] = 'i'; data[9] = '\''; data[10] = 's'; data[11] =
	     * '-'; data[12] = 'S'; data[13] = 't'; data[14] = 'a';
	     * data[15] = 'p'; data[16] = 'l'; data[17] = 'e'; data[18] =
	     * 'r'; data[19] = '>'; data[20] = '<'; data[21] = 'V';
	     * data[22] = '1'; data[23] = '1'; data[24] = '>'; data[25] =
	     * '<'; data[26] = 'C'; data[27] = '>'; data[28] = '<';
	     * data[29] = 'S'; data[30] = '='; data[31] = 'Q'; data[32] = ' 
	     * '; data[33] = '0'; data[34] = ' '; data[35] = '0'; data[36]
	     * = ' '; data[37] = '-'; data[38] = '.'; data[39] = '2';
	     * data[40] = ' '; data[41] = '.'; data[42] = '2'; data[43] = ' 
	     * '; data[44] = '.'; data[45] = '2'; data[46] = ' '; data[47]
	     * = '0'; data[48] = '>'; data[49] = '<'; data[50] = 'P';
	     * data[51] = '='; data[52] = '4'; data[53] = '7'; data[54] = ' 
	     * '; data[55] = '7'; data[56] = '5'; data[57] = ' '; data[58]
	     * = '6'; data[59] = '>'; data[60] = '<'; data[61] = '/';
	     * data[62] = 'C'; data[63] = '>'; data[64] = '<'; data[65] =
	     * 'S'; data[66] = '0'; data[67] = '>'; 
	     */

	    // strcpy((char *)VAR(current_msg),my_name);
	    // strncpy_P(data,my_name,48);
	    // strncpy(data,my_name,strlen(my_name));

	    TOS_CALL_COMMAND(UDP_ECHO_SUB_TOG_RED) ();	/* bang the green
							 * led */

	    if (TOS_CALL_COMMAND(UDP_ECHO_TX) (VAR(current_msg), dataLen)) {
		// if(TOS_CALL_COMMAND(UDP_ECHO_TX)(VAR(current_msg),6)){
		// if(TOS_CALL_COMMAND(UDP_ECHO_TX)(VAR(current_msg),strlen(my_name))){
		VAR(tx_pending) = 1;
	    }
	}			// else
    }

    /*
     * give back some buffer 
     */
    return ret_msg;
}

/*
 * This event signals the lower layer is done transmitting our message 
 */

char            TOS_EVENT(UDP_ECHO_TX_DONE) (TOS_MsgPtr tx_msg) {
    if (VAR(current_msg) == tx_msg) {
	VAR(tx_pending) = 0;
    }
    return EVENT_SUCCESS;
}
