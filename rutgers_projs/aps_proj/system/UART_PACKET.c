/*
 * tab:4 "Copyright (c) 2000 and The Regents of the University of
 * California.  All rights reserved. Permission to use, copy, modify, and 
 * distribute this software and its documentation for any purpose, without 
 * fee, and without written agreement is hereby granted, provided that the 
 * above copyright notice and the following two paragraphs appear in all
 * copies of this software. IN NO EVENT SHALL THE UNIVERSITY OF
 * CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
 * SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE UNIVERSITY OF
 * CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 * Authors: Jason Hill 
 */



#include "tos.h"
#include "UART_PACKET.h"


#undef FULLPC
#ifdef FULLPC
#include "Fullpc_uart_connect.h"
#endif


#define TOS_FRAME_TYPE UART_PACKET_obj_frame
TOS_FRAME_BEGIN(UART_PACKET_obj_frame)
{
    char            state;
    char           *send_ptr;
    char           *rec_ptr;
    TOS_Msg         buffer;
    char            count;
}
TOS_FRAME_END(UART_PACKET_obj_frame);



char            TOS_COMMAND(UART_PACKET_TX_PACKET) (TOS_MsgPtr msg) {
#ifndef FULLPC
    if (VAR(state) == 0) {
	VAR(send_ptr) = (char *) msg;
	VAR(count) = 1;
	VAR(state) = 1;
	if (TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES) (VAR(send_ptr)[0])) {
	    return 1;
	} else {
	    VAR(state) = 0;
	    VAR(count) = 0;
	    return 0;
	}
    } else {
	return 0;
    }
#else
    {
	int             i;
	printf("uart_send_packet\n");
	VAR(send_ptr) = (char *) msg;
	if (uart_send != 0) {
	    printf("sending packet: %d \n",
		   write(uart_send, VAR(send_ptr), sizeof(TOS_Msg)));
	}
	for (i = 0; i < sizeof(TOS_Msg); i++)
	    printf("%x,", VAR(send_ptr)[i]);
	printf("\n");
    }
    return 0;
#endif
}

void            TOS_COMMAND(UART_PACKET_POWER) (char mode) {
    // do this later;
    ;
}


char            TOS_COMMAND(UART_PACKET_INIT) () {
    VAR(state) = 0;
    VAR(rec_ptr) = (char *) &VAR(buffer);
#ifdef FULLPC
    udp_init_socket();
    printf("UART Packet handler initialized.\n");
#endif
    TOS_CALL_COMMAND(UART_PACKET_SUB_INIT) ();
    return 1;
}


char            TOS_EVENT(UART_PACKET_RX_BYTE_READY) (char data,
						      char error) {
#ifdef FULLPC_DEBUG
    printf("UART PACKET: byte arrived: %x, STATE: %d, COUNT: %d\n", data,
	   VAR(state), VAR(count));
#endif
    if (error) {
	VAR(state) = 0;
	return 0;
    }
    if (VAR(state) == 0) {
	VAR(state) = 5;
	VAR(count) = 1;
	VAR(rec_ptr)[0] = data;
    } else if (VAR(state) == 5) {
	VAR(rec_ptr)[(int) VAR(count)] = data;
	VAR(count)++;

	if (VAR(count) == sizeof(TOS_Msg)) {
	    TOS_MsgPtr      tmp;
	    VAR(state) = 0;
	    tmp =
		TOS_SIGNAL_EVENT(UART_PACKET_RX_PACKET_DONE) ((TOS_MsgPtr)
							      VAR
							      (rec_ptr));
	    if (tmp != 0)
		VAR(rec_ptr) = (char *) tmp;
	    return 0;
	}
    }
    return 1;
}

#ifndef FULLPC
char            TOS_EVENT(UART_PACKET_TX_BYTE_READY) (char success) {
    if (success == 0) {
	printf("TX_packet failed, TX_byte_failed");
	TOS_SIGNAL_EVENT(UART_PACKET_TX_PACKET_DONE) ((TOS_MsgPtr)
						      VAR(send_ptr));
	VAR(state) = 0;
	VAR(count) = 0;
    }
    // printf("State: %d Count: %d", VAR(state), VAR(count));
    if (VAR(state) == 1) {
	if (VAR(count) < sizeof(TOS_Msg)) {
	    TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES) (VAR(send_ptr)
							[(int)
							 VAR(count)]);
	    VAR(count)++;
	} else {
	    VAR(state) = 0;
	    VAR(count) = 0;
	    TOS_SIGNAL_EVENT(UART_PACKET_TX_PACKET_DONE) ((TOS_MsgPtr)
							  VAR(send_ptr));
	}
    }
    return 1;
}
#else
char            TOS_EVENT(UART_PACKET_TX_BYTE_READY) (char success) {
    return 0;
}
#endif
char            TOS_EVENT(UART_PACKET_BYTE_TX_DONE) () {
    return 1;
}

#ifdef FULLPC

void
uart_packet_evt()
{
    int             avilable;
    ioctl(uart_send, FIONREAD, &avilable);
    if (avilable > sizeof(TOS_Msg)) {
	read(uart_send, VAR(rec_ptr), sizeof(TOS_Msg));
	TOS_SIGNAL_EVENT(UART_PACKET_RX_PACKET_DONE) ((TOS_MsgPtr)
						      VAR(rec_ptr));
	printf("got packet\n");

    }

}



#endif
