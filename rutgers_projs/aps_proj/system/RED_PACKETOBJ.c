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
#include "RED_PACKETOBJ.h"

// #define FULLPC_DEBUG 1

char            error_check(char *ptr);


#define TOS_FRAME_TYPE PACKET_obj_frame
TOS_FRAME_BEGIN(PACKET_obj_frame)
{

    char            state;
    char            count;
    TOS_Msg         buffer;
    char           *rec_ptr;
    char           *send_ptr;
}
TOS_FRAME_END(PACKET_obj_frame);

TOS_TASK(check_error)
{
    TOS_MsgPtr      tmp;
    if (0 == error_check(VAR(rec_ptr))) {
	VAR(state) = 0;
	return;
    }
    VAR(state) = 0;
    tmp =
	TOS_SIGNAL_EVENT(PACKET_RX_PACKET_DONE) ((TOS_MsgPtr)
						 VAR(rec_ptr));
    if (tmp != 0)
	VAR(rec_ptr) = (char *) tmp;
}

char            TOS_COMMAND(PACKET_TX_PACKET) (TOS_MsgPtr msg) {
    if (VAR(state) == 0) {
	VAR(send_ptr) = (char *) msg;
	if (TOS_CALL_COMMAND(PACKET_SUB_TX_BYTES) (VAR(send_ptr)[0])) {
	    VAR(state) = 1;
	    VAR(count) = 1;
	    return 1;
	} else {
	    return 0;
	}
    } else {
	return 0;
    }
}

void            TOS_COMMAND(PACKET_POWER) (char mode) {
    // do this later;
    ;
}


char            TOS_COMMAND(PACKET_INIT) () {
    TOS_CALL_COMMAND(PACKET_SUB_INIT) ();
    VAR(state) = 0;
    VAR(rec_ptr) = (char *) &VAR(buffer);
#ifdef FULLPC
    printf("Packet handler initialized.\n");
#endif
    return 1;
}


char            TOS_EVENT(PACKET_RX_BYTE_READY) (char data, char error) {

#ifdef FULLPC_DEBUG
    printf("PACKET: byte arrived: %x, STATE: %d, COUNT: %d\n", data,
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
	    TOS_POST_TASK(check_error);
	    return 0;
	}
    }
    return 1;
}


char            TOS_EVENT(PACKET_TX_BYTE_READY) (char success) {
    if (success == 0) {
	printf("TX_packet failed, TX_byte_failed");
	TOS_SIGNAL_EVENT(PACKET_TX_PACKET_DONE) (0);
	VAR(state) = 0;
	VAR(count) = 0;
    }
    if (VAR(state) == 1) {
#ifdef FULLPC_DEBUG
	printf("PACKET: byte sent: %x, STATE: %d, COUNT: %d\n",
	       VAR(data)[(int) VAR(count)], VAR(state), VAR(count));
#endif
	if (VAR(count) < sizeof(TOS_Msg)) {
	    int             place = (int) VAR(count);
	    if (place > 29)
		place -= 30;
	    if (place > 29)
		place -= 30;
	    TOS_CALL_COMMAND(PACKET_SUB_TX_BYTES) (VAR(send_ptr)[place]);
	    VAR(count)++;
	} else if (VAR(count) == sizeof(TOS_Msg)) {
	    VAR(count)++;
	    return 0;
	} else {
	    VAR(state) = 0;
	    VAR(count) = 0;
	    TOS_SIGNAL_EVENT(PACKET_TX_PACKET_DONE) (1);
	    return 0;
	}
    }
    return 1;
}
char            TOS_EVENT(PACKET_BYTE_TX_DONE) () {
    return 1;
}

char
error_check(char *ptr)
{
    char            i = 0;
    char            j = 30;
    char            k = 60;
#ifdef FULLPC_DEBUG
    for (i = 0; i < sizeof(TOS_Msg); i++) {
	printf("%x, ", ptr[i]);
    }
    printf("\n");
#endif
    for (i = 0; i < 30; i++, j++, k++) {
	if (ptr[(int) i] == ptr[(int) j]) {;
	} else if (ptr[(int) i] == ptr[(int) k]) {;
	} else if (ptr[(int) j] == ptr[(int) k])
	    ptr[(int) i] = ptr[(int) j];
	else
	    return 0;
    }
    return 1;
}
