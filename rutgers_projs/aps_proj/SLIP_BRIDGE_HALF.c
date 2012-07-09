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
 * Version:                     $Id: SLIP_BRIDGE_HALF.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:               Thu Aug  9 16:48:21 2001
 * Filename:                    SLIP_BRIDGE_HALF.c
 * History:
 * $Log: SLIP_BRIDGE_HALF.c,v $
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
 * Revision 1.1.1.1  2001/10/04 04:12:06  rmartin
 * hopefully this is correct
 *
 * Revision 1.1  2001/08/10 15:00:14  rmartin
 * half-duplex bridge allows for larger packets
 *
 */


/*
 * A simple shunt that connects the SLIP driver on the UART side to the
 * GENERIC_COMM component. Also connects the generic comm to the SLIP
 * driver.  This is different from SLIP_BRIDGE in that it is
 * half-duplex.  Theory of operation: The driver is a classic buffer
 * swapper type of driver for an event-driven system (like TOS). Each
 * low-level driver (SLIP and Generic_comm), has a single RX buffer.
 * Unlike Slip_bridge, this highest level component, in this case the
 * SLIP_BRIDGE_HALF, maintains only a single message buffer. On an RX
 * from either side, the SLIP_BRIDGE swaps its single buffer for the
 * driver's buffer.  If the other driver completes a receive, and hands
 * up the bridge a buffer while the TX buffer is busy, we end up returning 
 * the same buffer without moving it to the other driver. The
 * SLIP_BRIDGE_HALF modifies the first word of the message over the radio 
 * to include the length field from the SLIP driver. You have to
 * compile with different destination addresses to make the bridge work.
 * 
 */

#include "tos.h"
// #define FULLPC_DEBUG 1
#include "SLIP_BRIDGE_HALF.h"

#ifndef BRIDGE_DEST
#error A bridge destination must be defined in BRIDGE_DEST for SLIP_BRIDGE_HALF.c
#endif

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILIED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILIED 0

/*
 * ------------ Frame Declaration ---------- 
 */
#define TOS_FRAME_TYPE SLIP_BRIDGE_HALF_frame
TOS_FRAME_BEGIN(SLIP_BRIDGE_HALF_frame)
{
    TOS_MsgPtr      tx_msg;	/* ptr to msg "owned" by the bridge for
				 * radio */
    TOS_Msg         tx_buffer;	/* a base message buffer */
    char            tx_pending;	/* we have an outstanding TX message to
				 * either the uart or radio */
}
TOS_FRAME_END(SLIP_BRIDGE_HALF_frame);


/*
 * ------------------ Initialization section --------------------
 */
char            TOS_COMMAND(SLIP_BRIDGE_HALF_INIT) () {

    VAR(tx_msg) = (TOS_MsgPtr) & (VAR(tx_buffer));

    VAR(tx_pending) = 0;

    /*
     * init the radio side then the slip side 
     */
    if (TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_RADIO_INIT) () ==
	HANDSHAKE_FAILIED) {
	return HANDSHAKE_FAILIED;
    }

    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_LED_INIT) ();

    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_GREEN) ();

    return (TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_SLIP_INIT) ());
}

/*
 * don't do anything here (yet) 
 */
char            TOS_COMMAND(SLIP_BRIDGE_HALF_START) () {
    return HANDSHAKE_SUCCESS;
}

/*
 * another power-mode command 
 */
char            TOS_COMMAND(SLIP_BRIDGE_HALF_POWER) (char mode) {

    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_SLIP_POWER) (mode);
    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_RADIO_POWER) (mode);

    return HANDSHAKE_SUCCESS;
}

/*
 * ------------------------ Radio side ---------------------------
 */

/*
 * handle an incomming message from the radio 
 */
TOS_MsgPtr      TOS_MSG_EVENT(SLIP_BRIDGE_HALF_RX_RADIO) (TOS_MsgPtr
							  rx_msg) {
    TOS_MsgPtr      ret_msg;
    int             i,
                    len;

    ret_msg = rx_msg;		/* assume we're giving back the original
				 * buffer */

    if (VAR(tx_pending) == 0) {	/* if any transmit is pending, drop this
				 * message */

	TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_RED) ();

	/*
	 * swap the bridge's single TX buffer with the radio's RX buffer 
	 */
	ret_msg = VAR(tx_msg);
	VAR(tx_msg) = rx_msg;

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
	 * call the slip side to TX the message 
	 */
	if (TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_TX_SLIP) (VAR(tx_msg), len)) {

	    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_RED) ();	/* bang
								 * the red 
								 * led */
	    VAR(tx_pending) = 1;
	}
    }

    /*
     * give back some buffer to the radio 
     */
    return ret_msg;
}

/*
 * This event signals the radio TX is done, which was caused by the the
 * SLIP RX event 
 */
char            TOS_EVENT(SLIP_BRIDGE_HALF_TX_RADIO_DONE) (TOS_MsgPtr
							   tx_done_msg) {

    if (VAR(tx_msg) == tx_done_msg) {
	/*
	 * bang the green led 
	 */
	TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_YELLOW) ();
	VAR(tx_pending) = 0;
    }

    return EVENT_SUCCESS;
}

/*
 * ------------------------ SLIP side ----------------------------
 */
TOS_MsgPtr      TOS_EVENT(SLIP_BRIDGE_HALF_RX_SLIP) (TOS_MsgPtr rx_msg,
						     int len) {

    TOS_MsgPtr      ret_msg;
    int             i;

    ret_msg = rx_msg;		/* assume we're giving back the original
				 * buffer */

    if (VAR(tx_pending) == 0) {

	TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_YELLOW) ();
	/*
	 * swap the bridge's single TX buffer with the SLIP driver's RX
	 * buffer 
	 */

	ret_msg = VAR(tx_msg);
	VAR(tx_msg) = rx_msg;
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
	 * call the radio side to TX the message 
	 */
	if (TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_TX_RADIO) (BRIDGE_DEST,
							 AM_MSG
							 (SLIP_BRIDGE_HALF_RX_RADIO),
							 VAR(tx_msg))) {
	    VAR(tx_pending) = 1;
	    /*
	     * bang the green led 
	     */
	    TOS_CALL_COMMAND(SLIP_BRIDGE_HALF_SUB_TOG_GREEN) ();
	}
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
char            TOS_EVENT(SLIP_BRIDGE_HALF_TX_SLIP_DONE) (TOS_MsgPtr
							  tx_done_msg) {

    if (VAR(tx_msg) == tx_done_msg) {
	VAR(tx_pending) = 0;
    }

    return EVENT_SUCCESS;
}
