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
#include "CRCPACKETOBJ.h"
#define FULLPC_DEBUG 1

int             calcrc(char *ptr, int count);


#define TOS_FRAME_TYPE PACKET_obj_frame
TOS_FRAME_BEGIN(PACKET_obj_frame)
{

    char            state;	/* 0: RCV, 1: TX, 2: STNDBY */
    char           *data;	/* send data bufer */
    char           *msg;
    TOS_Msg         recbuf;
    char            count;
}
TOS_FRAME_END(PACKET_obj_frame);

TOS_TASK(CRC_calc)
{
    ((TOS_MsgPtr) VAR(data))->crc = calcrc(VAR(data), sizeof(TOS_Msg) - 2);
#ifdef FULLPC_DEBUG
    printf("CRC: %x\n", ((TOS_MsgPtr) VAR(data))->crc);
#endif
}

TOS_TASK(check_crc)
{
    int             crc = calcrc(VAR(msg), sizeof(TOS_Msg) - 2);
    VAR(state) = 0;
    if (crc == ((TOS_MsgPtr) VAR(msg))->crc) {
	TOS_MsgPtr      tmp;
	tmp =
	    TOS_SIGNAL_EVENT(PACKET_RX_PACKET_DONE) ((TOS_MsgPtr)
						     VAR(msg));
	if (tmp != 0)
	    VAR(msg) = (char *) tmp;
#ifdef FULLPC
    } else {
	printf("crc check failed: %x, %x\n", crc,
	       ((TOS_MsgPtr) VAR(msg))->crc);
#endif
    }
}

char            TOS_COMMAND(PACKET_TX_PACKET) (TOS_MsgPtr data) {
    if (VAR(state) == 0) {	/* receiving */
	VAR(data) = (char *) data;
#ifdef FULLPC_DEBUG
	printf("PACKET: byte sent: %x, STATE: %d, COUNT: %d\n",
	       VAR(data)[0] & 0xff, VAR(state), 0);
#endif
	if (TOS_CALL_COMMAND(PACKET_SUB_TX_BYTES) (VAR(data)[0])) {	/* start 
									 * tx 
									 */
	    TOS_POST_TASK(CRC_calc);
	    VAR(state) = 1;	/* transmitting */
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
    VAR(msg) = (char *) &VAR(recbuf);
    VAR(state) = 0;
#ifdef FULLPC
    printf("Packet handler initialized.\n");
#endif
    return 1;
}

char            TOS_EVENT(PACKET_RX_BYTE_READY) (char data, char error) {

    printf("PACKET: byte arrived: %x, STATE: %d, COUNT: %d\n", data,
	   VAR(state), VAR(count));
    if (error) {
	VAR(state) = 0;
	return 0;
    }
    if (VAR(state) == 0) {
	VAR(state) = 5;
	VAR(count) = 1;
	VAR(msg)[0] = data;
    } else if (VAR(state) == 5) {
	VAR(msg)[(int) VAR(count)] = data;
	VAR(count)++;
	if (VAR(count) == sizeof(TOS_Msg)) {
	    TOS_POST_TASK(check_crc);
	    return 0;
	}
    }
    return 1;
}


/*
 * PACKET_TX_BYTE_READY: spool bytes to packet component in event-driven
 * manner. On each byte event, return the next byte till done. 
 */
char            TOS_EVENT(PACKET_TX_BYTE_READY) (char success) {
    if (success == 0) {
	printf("TX_packet failed, TX_byte_failed");
	TOS_SIGNAL_EVENT(PACKET_TX_PACKET_DONE) ((TOS_MsgPtr) VAR(msg));
	VAR(state) = 0;
	VAR(count) = 0;
    }
    if (VAR(state) == 1) {
	if (VAR(count) < sizeof(TOS_Msg)) {
#ifdef FULLPC_DEBUG
	    printf("PACKET: byte sent: %x, STATE: %d, COUNT: %d\n",
		   VAR(data)[(int) VAR(count)] & 0xff, VAR(state),
		   VAR(count));
#endif
	    TOS_CALL_COMMAND(PACKET_SUB_TX_BYTES) (VAR(data)
						   [(int) VAR(count)]);
	    VAR(count)++;
	} else if (VAR(count) == sizeof(TOS_Msg)) {
	    VAR(count)++;
	    return 0;
	} else {
	    VAR(state) = 0;
	    VAR(count) = 0;
	    TOS_SIGNAL_EVENT(PACKET_TX_PACKET_DONE) ((TOS_MsgPtr)
						     VAR(data));
	    return 0;
	}
    }
    return 1;
}
char            TOS_EVENT(PACKET_BYTE_TX_DONE) () {
    return 1;
}

int
calcrc(char *ptr, int count)
{
    short           crc;
    char            i;

    crc = 0;
    while (--count >= 0) {
	crc = crc ^ (int) *ptr++ << 8;
	i = 8;
	do {
	    if (crc & 0x8000)
		crc = crc << 1 ^ 0x1021;
	    else
		crc = crc << 1;
	} while (--i);
    }
    return (crc);
}
