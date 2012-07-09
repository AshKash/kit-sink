/*
 * tab:4 AMP - AM Permiscuous This module is intended to be used for
 * packet sniffing purposes. It ignores the destination and group
 * addresses of incoming packets. This module is AM without the
 * destination check - intended for the purpose of packet sniffing.
 * "Copyright (c) 2000 and The Regents of the University of California.
 * All rights reserved. Permission to use, copy, modify, and distribute
 * this software and its documentation for any purpose, without fee, and
 * without written agreement is hereby granted, provided that the above
 * copyright notice and the following two paragraphs appear in all copies
 * of this software. IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE
 * LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE. THE UNIVERSITY OF CALIFORNIA
 * SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND
 * THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS." Authors: Jason Hill 
 */



// This is an AM messaging layer implementation that understands multiple
// output devices.  All packets addressed to 0x7e are sent to the UART
// instead of the radio.


#ifdef FULLPC
#define FULLPC_DEBUG 1
#endif


// extern const char LOCAL_ADDR_BYTE_1;
extern const unsigned char TOS_LOCAL_ADDRESS;

#include "tos.h"
#include "AM.h"




#define TOS_FRAME_TYPE AM_obj_frame
TOS_FRAME_BEGIN(AM_obj_frame)
{
    char            addr;
    char            type;
    char            state;
    TOS_MsgPtr      msg;

}
TOS_FRAME_END(AM_obj_frame);


// static inline TOS_MsgPtr TOS_EVENT(AM_NULL_FUNC)(TOS_MsgPtr
// data){return data;} /* Signal data event to upper comp */
TOS_MsgPtr      AM_MSG_REC(char num, TOS_MsgPtr data);

TOS_TASK(AM_send_task)
{
    // construct the packet to be sent.
    // fill in dest and type


    // TOS_CALL_COMMAND(NM_LEDS_GREEN_OFF)();


    if (!TOS_CALL_COMMAND(AM_SUB_TX_PACKET) (VAR(msg))) {
	TOS_SIGNAL_EVENT(AM_MSG_SEND_DONE) (VAR(msg));
	VAR(state) = 0;
	return;
    }
}



char            TOS_COMMAND(AM_SEND_MSG) (char addr, char type,
					  TOS_MsgPtr data) {

    // if not idle then return error.
    if (VAR(state) == 0) {
	// schedule sending task.

	// TOS_CALL_COMMAND(NM_LEDS_GREEN_ON)();

	TOS_POST_TASK(AM_send_task);
	// set state to waiting to send.
	VAR(state) = 1;
	// store away important information.

	VAR(msg) = data;
	VAR(msg)->addr = addr;
	VAR(msg)->type = type;
	VAR(msg)->group = LOCAL_GROUP & 0xff;
#ifdef FULLPC_DEBUG
	{
	    int             i;
	    printf("Sending message: %x, %x\n\t", addr, type);
	    for (i = 0; i < sizeof(TOS_Msg); i++)
		printf("%x,", ((char *) data)[i]);
	    printf("::%d\n", sizeof(TOS_Msg));
	}
#endif
	return 1;
    }
    return 0;
}

char            TOS_COMMAND(AM_POWER) (char mode) {
    TOS_CALL_COMMAND(AM_SUB_POWER) (mode);
    VAR(state) = 0;
    return 1;

}

char            TOS_COMMAND(AM_INIT) () {
    printf("AM Module initialized\n");
    // initialize sub components
    TOS_CALL_COMMAND(AM_SUB_INIT) ();
    VAR(state) = 0;
    return 1;
}

char            TOS_EVENT(AM_TX_PACKET_DONE) (TOS_MsgPtr msg) {
    // return to idle state.
    VAR(state) = 0;
    // fire send done event.
    TOS_SIGNAL_EVENT(AM_MSG_SEND_DONE) (msg);
    return 1;
}

TOS_MsgPtr      TOS_EVENT(AM_RX_PACKET_DONE) (TOS_MsgPtr packet) {
#ifdef FULLPC
    int             i;
    printf("AM_addres = %x, %x\n", packet->addr, packet->type);
#endif


    // here is the permiscuous code

    // when packet is received check if it is broadcast or addressed
    // to local address.
    // if(packet->group == (LOCAL_GROUP & 0xff) && (packet->addr ==
    // (char)TOS_BCAST_ADDR || packet->addr == TOS_LOCAL_ADDRESS)){

    {
	TOS_MsgPtr      tmp;
#ifdef FULLPC_DEBUG
	printf("Received message:\n\t");
	for (i = 0; i < 30; i++)
	    printf("%x,", ((char *) packet)[i]);
	printf("\n");
	printf("AM_type = %d\n", packet->type);
#endif
	// send message to be dispatched.

	tmp = AM_MSG_REC(packet->type, packet);
	if (tmp != 0) {
	    return (TOS_MsgPtr) tmp;
	} else {
	    return packet;
	}
    }
    return packet;
}

#include "AMdispatch.template"
