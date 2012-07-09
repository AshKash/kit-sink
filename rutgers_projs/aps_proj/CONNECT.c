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


// this components explores routing topology and then broadcasts back
// light readings.

#include "tos.h"
#include "CONNECT.h"

#define TOS_FRAME_TYPE CONNECT_obj_frame
TOS_FRAME_BEGIN(CONNECT_obj_frame)
{
    char            route;
    char            set;
    TOS_Msg         route_buf;
    TOS_Msg         data_buf;
    TOS_MsgPtr      msg;
    char            data_send_pending;
    char            msg_send_pending;
    char            place;
    int             prev;
    char            count;
}
TOS_FRAME_END(CONNECT_obj_frame);

char            TOS_COMMAND(CONNECT_INIT) () {
    // initialize sub components
    TOS_CALL_COMMAND(CONNECT_SUB_INIT) ();
    VAR(msg) = &VAR(route_buf);
    VAR(data_send_pending) = 0;
    VAR(msg_send_pending) = 0;
#ifdef BASE_STATION
    // set beacon rate for route updates to be sent
    TOS_COMMAND(CONNECT_SUB_CLOCK_INIT) (255, 0x06);
    printf("base route set to TOS_UART_ADDR\n");
    // route to base station is over UART.
    VAR(route) = TOS_UART_ADDR;
    VAR(set) = 1;
    VAR(place) = 0;
    VAR(data_buf).data[0] = 1;
    VAR(data_buf).data[1] = TOS_LOCAL_ADDRESS;
#else
    // set rate for sampling.
    TOS_COMMAND(CONNECT_SUB_CLOCK_INIT) (255, 0x03);
    VAR(set) = 0;
    VAR(route) = 0;
    VAR(count) = 0;
#endif
    return 1;
}

char            TOS_COMMAND(CONNECT_START) () {
    return 1;
}

void
update_connections(char source, char strength)
{
    // first off, update the local state to reflect being able to 
    // hear this sender.
    VAR(data_buf).data[8 + VAR(place)] = source;
    VAR(data_buf).data[8 + VAR(place) + 1] = strength;
    VAR(place) = (VAR(place) + 2) & 0xf;
}

// This handler responds to routing updates.
TOS_MsgPtr      TOS_MSG_EVENT(CONNECT_UPDATE) (TOS_MsgPtr msg) {
    TOS_MsgPtr      tmp;
    char           *data = msg->data;

    // clear LED2 when update is received.
    TOS_CALL_COMMAND(CONNECT_LED2_OFF) ();

    update_connections(data[(int) data[0]], data[28]);
    // if route hasn't already been set this period...
    if (VAR(set) == 0) {
	// record route
	VAR(route) = data[(int) data[0]];
	VAR(set) = 8;
	data[0]++;
	// create a update packet to be sent out.
	data[(int) data[0]] = TOS_LOCAL_ADDRESS;
	// send the update packet.
	if (VAR(msg_send_pending) == 0) {
	    VAR(msg_send_pending) =
		TOS_CALL_COMMAND(CONNECT_SUB_SEND_MSG) (TOS_BCAST_ADDR,
							AM_MSG
							(CONNECT_UPDATE),
							msg);
	} else {
	    return msg;
	}
	printf("route set to %x\n", VAR(route));
	tmp = VAR(msg);
	VAR(msg) = msg;
	return tmp;

    } else {
	printf("route already set to %x\n", VAR(route));
	return msg;
    }
}

TOS_MsgPtr      TOS_MSG_EVENT(DATA_MSG) (TOS_MsgPtr msg) {
    char           *data = msg->data;
    TOS_MsgPtr      tmp = msg;
    char            source;
    // this handler forwards packets traveling to the base.
    TOS_CALL_COMMAND(CONNECT_LED2_OFF) ();
    // if this is the first hop on the route, then use the origin of the
    // packet< 
    // as the name of the person who sent the packet to you instead of the 
    // 
    // 
    // last hop ID (which is null)
    source = data[2];
    if (source == 0) {
	source = data[0];
    }
    update_connections(source, data[28]);
    // if a route is known, forward the packet towards the base.
    if (VAR(route) != 0 && data[1] == TOS_LOCAL_ADDRESS) {
#ifdef BASE_STATION
	if (VAR(msg_send_pending) == 0) {
	    VAR(msg_send_pending) =
		TOS_CALL_COMMAND(CONNECT_SUB_SEND_MSG) (TOS_UART_ADDR,
							AM_MSG(DATA_MSG),
							msg);
	    tmp = VAR(msg);
	    VAR(msg) = msg;
	}
#else
	// update the packet.
	data[5] = data[4];
	data[4] = data[3];
	data[3] = data[2];
	data[2] = data[1];
	data[1] = VAR(route);
	// send the packet.
	if (VAR(msg_send_pending) == 0) {
	    VAR(msg_send_pending) =
		TOS_CALL_COMMAND(CONNECT_SUB_SEND_MSG) (TOS_BCAST_ADDR,
							AM_MSG
							(CONNECT_UPDATE),
							msg);
	    tmp = VAR(msg);
	    VAR(msg) = msg;
	}
#endif
	printf("routing to home %x\n", VAR(route));
    }
    return tmp;
}


void            TOS_EVENT(CONNECT_SUB_CLOCK) () {
    // clear LED3 when the clock ticks.
    TOS_CALL_COMMAND(CONNECT_LED3_OFF) ();
    printf("route clock\n");
#ifdef BASE_STATION
    // if is the base, then it should send out the route update.
    if (VAR(data_send_pending) == 0) {
	VAR(data_send_pending) =
	    TOS_CALL_COMMAND(CONNECT_SUB_SEND_MSG) (TOS_BCAST_ADDR,
						    AM_MSG(CONNECT_UPDATE),
						    &VAR(data_buf));
    }
#else
    // decrement the set var to know when a period is over.
    if (VAR(set) != 0)
	VAR(set)--;
    // read the value from the sensor.
    TOS_COMMAND(CONNECT_SUB_READ) ();
#endif
    TOS_CALL_COMMAND(CONNECT_LED1_TOGGLE) ();

}


char            TOS_EVENT(CONNECT_SUB_DATA_READY) (int data) {
    // when the data comes back from the sensor, see if the counter
    // has expired or if the value is significantly different
    // from the previous value.
    if (VAR(route) != 0 &&
	((VAR(prev) - data) > 100 ||
	 (data - VAR(prev)) > 100 || VAR(count) > 10)) {
	char           *buf = VAR(data_buf).data;
	// if a new data packet needs to be sent, go for it.
	if (VAR(data_send_pending) == 0) {
	    buf[6] = data >> 8;
	    buf[7] = data & 0xff;
	    buf[0] = TOS_LOCAL_ADDRESS;
	    buf[1] = VAR(route);
	    VAR(data_send_pending) =
		TOS_CALL_COMMAND(CONNECT_SUB_SEND_MSG) (TOS_BCAST_ADDR,
							AM_MSG(DATA_MSG),
							&VAR(data_buf));
	    VAR(count) = 0;
	    // blink the LED
	    TOS_CALL_COMMAND(CONNECT_LED3_ON) ();
	}
    }
    // increment the counter and store the previous reading.
    VAR(prev) = data;
    VAR(count)++;
    return 1;
}

char            TOS_EVENT(CONNECT_SEND_DONE) (TOS_MsgPtr data) {
    if (data == VAR(msg))
	VAR(msg_send_pending) = 0;
    if (data == &VAR(data_buf))
	VAR(data_send_pending) = 0;
    return 1;
}
