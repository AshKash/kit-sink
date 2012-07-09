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
 * Version:                     $Id: SLIP.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:               Fri Jul 13 11:01:53 2001
 * Filename:                    SLIP.c
 * History:
 * $Log: SLIP.c,v $
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
 * Revision 1.4  2001/09/28 22:25:29  xili
 * some fix for slip_bridge to run on the simulator, change only in FULLPC mode
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
 * this is a generic SLIP driver for the UART. Tries hard to conform to
 * RFC 1055. On the TX side, accepts TOS messages, encodes them as SLIP 
 * packets while sending them out the UART on the RX side, it
 * de-encapsulates the SLIP messages and then sends them up to a higher
 * layer. The TX and RX states are managed separately so that
 * full-duplex operation is possible. 
 */

#include "tos.h"
// #define FULLPC_DEBUG 1
#include "SLIP.h"

/*
 * SLIP special character codes these are from the RFC 1055 
 */
#define END     ((unsigned char)0xc0)	/* start/end delimiter for packets 
					 */
#define ESC     ((unsigned char)0xdb)	/* indicates byte stuffing */
#define ESC_END ((unsigned char)0xdc)	/* ESC ESC_END means END data byte 
					 */
#define ESC_ESC ((unsigned char)0xdd)	/* ESC ESC_ESC means ESC data byte 
					 */

/*
 * enable PC debugging 
 */
#ifdef FULLPC
#include "Fullpc_uart_connect.h"
#endif

#define BAUD 19200

/*
 * general TOSy defines 
 */
#define HANDSHAKE_SUCCESS 1
#define HANDSHAKE_FAILED 0
#define EVENT_SUCCESS 1
#define EVENT_FAILED 0
#define NULL         0

/*
 * SLIP component states 
 */
#define IDLE           0	/* nothing going on */

#define TX_MASK        3	/* TX state bits */
#define TRANSMITTING   1	/* we are transmitting */
#define ESCAPING_TX    2	/* escape the next byte */


#define RX_MASK        0xc	/* rx state bits */
#define RECEIVING      4	/* we are receiving */
#define ESCAPING_RX    8	/* got an escaped previous byte */

/*
 * ------------ Frame Declaration ---------- 
 */
#define TOS_FRAME_TYPE SLIP_frame
TOS_FRAME_BEGIN(SLIP_frame)
{
    char            state;	/* transmitting, receiving, escaping, or
				 * nothing? */
    TOS_MsgPtr      tx_ptr;	/* transmit (out to UART) message pointer */
    TOS_MsgPtr      rx_ptr;	/* receive (in from UART) message pointer */
    TOS_Msg         bottomBuf;	/* the device driver holds the init RX
				 * buffer */
    char            tx_len;	/* length of the packet to transmit */
    char            encodePos;	/* the encoding position */
    char            decodePos;	/* the decoding position */
}
TOS_FRAME_END(SLIP_frame);


/*
 * ------------------ Initialization section --------------------
 */

char            TOS_COMMAND(SLIP_INIT) () {
    VAR(state) = IDLE;
    VAR(tx_len) = 0;
    VAR(encodePos) = 0;
    VAR(decodePos) = 0;

    VAR(rx_ptr) = (TOS_MsgPtr) & VAR(bottomBuf);
    VAR(tx_ptr) = (TOS_MsgPtr) NULL;

#ifdef FULLPC
    /*
     * Anyway, TOS_CALL_COMMAND(SLIP_SUB_INIT)((unsigned int)BAUD) will
     * call udp_init_socket() in the FULLPC state 
     */
    // udp_init_socket();
    printf("SLIP: UART handler init \n");
#endif
    return (TOS_CALL_COMMAND(SLIP_SUB_INIT) ((unsigned int) BAUD));
}

/*
 * no start code for now 
 */
char            TOS_COMMAND(SLIP_START) () {

    return HANDSHAKE_SUCCESS;
}

/*
 * for some reason, most of the power commands return void. Does this
 * violate the TOS command-handshake protocol? 
 */
char            TOS_COMMAND(SLIP_POWER) (char mode) {
    /*
     * no power-mode code for now, call UART power mode 
     */

    return (TOS_CALL_COMMAND(SLIP_SUB_POWER) (mode));
}

/*
 * ------------------ Transmit side --------------------
 */

/*
 * This TX inmterface deviates from the standard TOS fixed packet length
 * model; we require the higher layer to give us a length using this
 * interface because SLIP assumes variable length packets 
 */
char            TOS_COMMAND(SLIP_TX_PACKET) (TOS_MsgPtr message, int len) {

#ifndef FULLPC

    /*
     * if we're not in a transmit state then ... 
     */
    if (!(VAR(state) & TX_MASK)) {

	VAR(state) |= TRANSMITTING;

	/*
	 * ignore the TOS headers, send only the data part 
	 */
	VAR(tx_ptr) = message;
	VAR(tx_len) = len;
	VAR(encodePos) = 0;	/* initial position */

	/*
	 * send the first byte, an END char 
	 */
	if (TOS_CALL_COMMAND(SLIP_SUB_TX_BYTE) (END)) {
	    return HANDSHAKE_SUCCESS;
	} else {
	    /*
	     * failed to TX the byte. Bad. abort the whole packet. 
	     */
	    VAR(state) = IDLE;
	    VAR(encodePos) = 0;
	    return HANDSHAKE_FAILED;
	}
    } else {
	/*
	 * called by the higher layer either too fast of wrong. Abort the
	 * current transmission, reception and reset. Not clear if this is
	 * the right thing to do or wait for TX to finish then pay
	 * attention to a higher layer handshake fault 
	 */
	VAR(state) = IDLE;
	VAR(encodePos) = 0;
	return HANDSHAKE_FAILED;
    }
#else
    {
	int             i;
	printf("uart_send_packet\n");
	VAR(tx_ptr) = message;
	if (uart_send != 0) {
	    /*
	     * should send only the data 
	     */
	    // printf("sending packet: %d \n", write(uart_send,
	    // VAR(tx_ptr), len));
	    printf("sending packet: %d \n",
		   write(uart_send, VAR(tx_ptr)->data, len));
	}
	for (i = 0; i < sizeof(TOS_Msg); i++)
	    printf("%x,", VAR(tx_ptr)->data[i]);
	printf("\n");
    }
    return 0;
#endif
}				/* end SLIP_TX_PACKET */

/*
 * this function handles outgoing bytes to the UART. it assumes the first 
 * byte is sent, then encodes the rest using 8-bit slip style encoding 
 */

char            TOS_EVENT(SLIP_TX_BYTE_READY) (char success) {
    unsigned char   c;

    /*
     * we had better be in some transmitting state here 
     */
    if ((VAR(state) & TX_MASK)) {

	/*
	 * final ESC sent. and done transmitting the whole packet 
	 */
	if (VAR(encodePos) > VAR(tx_len)) {
	    int             tmp;

	    /*
	     * done TXing the packet. Reset state to not Tx 
	     */
	    VAR(state) &= (~TX_MASK);
	    /*
	     * give up the TX pointer here too 
	     */
	    VAR(tx_len) = 0;

	    /*
	     * signal completion of the event 
	     */
	    tmp = TOS_SIGNAL_EVENT(SLIP_TX_PACKET_DONE) (VAR(tx_ptr));
	    VAR(tx_ptr) = NULL;
	    return tmp;

	} else if (VAR(encodePos) == VAR(tx_len)) {

	    VAR(encodePos)++;
	    /*
	     * send the final ESC packet terminator 
	     */
	    return (TOS_CALL_COMMAND(SLIP_SUB_TX_BYTE) (END));

	} else {		/* send a data char */

	    c = (unsigned char) VAR(tx_ptr)->data[(int) VAR(encodePos)];
	    /*
	     * escaped the previous character? 
	     */
	    if (VAR(state) & ESCAPING_TX) {

		switch (c) {	/* TX the right char */
		case ESC:
		    c = (unsigned char) ESC_ESC;
		case END:
		    c = (unsigned char) ESC_END;
		default:
		    /*
		     * this should not happen 
		     */
		    break;
		}		/* end switch */

		/*
		 * return the state to a non-escape mode 
		 */
		VAR(state) &= (~ESCAPING_TX);	/* zero out TX bits */

	    } else {		/* we're in regular TX mode */

		/*
		 * is this a funny character? 
		 */
		if ((c == ESC) || (c == END)) {
		    VAR(state) |= ESCAPING_TX;
		    c = ESC;
		    VAR(encodePos)--;	/* hack: "back-up" and advance
					 * later */
		}
	    }			/* end regular data TX mode */

	    /*
	     * common TX clean-up code 
	     */
	    /*
	     * advance the position pointer 
	     */
	    VAR(encodePos)++;

	    /*
	     * send the single byte of data 
	     */
	    return (TOS_CALL_COMMAND(SLIP_SUB_TX_BYTE) (c));

	}			/* end send regular data */
    } else {
#ifdef FULLPC
	/*
	 * in simulation state, we never use this function to send packet.
	 * So, should ignore it in order to avoid interference with the
	 * receiving 
	 */
	return HANDSHAKE_SUCCESS;
#endif
	/*
	 * Not in TX mode? Whoa nally! Try to reset to recover 
	 */
	VAR(state) = IDLE;
	VAR(encodePos) = 0;
	return HANDSHAKE_FAILED;
    }

}				/* end SLIP_TX_BYTE_READY */

/*
 * bogus event to handle wiring correctly 
 */
char            TOS_EVENT(SLIP_BYTE_TX_DONE) (void) {

    return EVENT_SUCCESS;
}
/*
 * ------------------ Receive side --------------------
 */

/*
 * send a packet up to higher-level TOS component and reset the state to
 * idle 
 */
char
rx_and_reset()
{
    TOS_MsgPtr      next_rx_ptr;

    /*
     * set the state to not receiving 
     */
    VAR(state) &= ~RX_MASK;

    next_rx_ptr =
	(TOS_MsgPtr) TOS_SIGNAL_EVENT(SLIP_RX_PACKET_DONE) (VAR(rx_ptr),
							    VAR
							    (decodePos));

    if ((next_rx_ptr != (TOS_MsgPtr) NULL)) {
	int             i;

	/*
	 * don't zero out this buffer unless we actually swapped the
	 * buffers 
	 */
	if (VAR(rx_ptr) != next_rx_ptr) {
	    VAR(rx_ptr) = next_rx_ptr;
	    /*
	     * zero out the data in the packet 
	     */
	    for (i = 0; i < DATA_LENGTH; i++) {
		next_rx_ptr->data[i] = 0;
	    }
	}

	return EVENT_SUCCESS;
    } else {			/* No rx_pointer? Oh dear, this should not 
				 * happen! */
	return EVENT_FAILED;
    }

}				/* end rx_and_reset */

char            TOS_EVENT(SLIP_RX_BYTE_READY) (char d, char err) {
    char            c;
    unsigned char   data;

    /*
     * need to work around this somehow 
     */

    data = (unsigned char) d;

    if (err) {
	VAR(state) = IDLE;
	return EVENT_FAILED;
    }
    /*
     * not in an RX state 
     */
    if (!(VAR(state) & RX_MASK)) {

	if (data != END) {
	    return EVENT_SUCCESS;	/* not a slip packet? */
	}

	VAR(state) |= RECEIVING;	/* we are receiving now */
	VAR(decodePos) = 0;

	return EVENT_SUCCESS;

    } else if ((VAR(state) & RX_MASK) == RECEIVING) {

	switch (data) {
	case END:
	    return (rx_and_reset());
	    break;
	case ESC:
	    VAR(state) |= ESCAPING_RX;
	    break;

	    /*
	     * Oh hum, a data byte. Just stuff it in 
	     */
	default:
	    if (VAR(decodePos) < DATA_LENGTH) {
		VAR(rx_ptr)->data[(int) VAR(decodePos)] = data;
		VAR(decodePos)++;
	    } else {		/* a packet is > than the MTU! Truncate
				 * the packet as per RFC 1055 */
		return (rx_and_reset());
	    }
	}			/* end switch for general receive */

	return EVENT_SUCCESS;
    } else if ((VAR(state) & ESCAPING_RX)) {	/* in escape mode */
	c = data;
	switch (data) {

	case ESC_END:		/* this is the end of the packet */
	    c = END;
	    break;
	case ESC_ESC:		/* we sent an escape char */
	    /*
	     * change the data to the escape character 
	     */
	    c = ESC;
	default:
	    /*
	     * if "data" is not one of these two, then there is a
	     * protocol violation.  the RFC 1055 code leaves the byte
	     * alone and just stuffs it into the packet 
	     */
	    break;
	}			/* end switch on data */

	/*
	 * set the state back to normal RX 
	 */
	VAR(state) &= (~RX_MASK) | RECEIVING;

	if (VAR(decodePos) < DATA_LENGTH) {
	    VAR(rx_ptr)->data[(int) VAR(decodePos)] = c;
	    VAR(decodePos)++;
	} else {		/* buffer overrun, send the packet and
				 * abort */
	    return (rx_and_reset());
	}
	return EVENT_SUCCESS;
    } else {
	/*
	 * should never get here 
	 */
	VAR(state) = IDLE;
	return EVENT_FAILED;
    }

}				/* end SLIP_RX_BYTE_READY */
