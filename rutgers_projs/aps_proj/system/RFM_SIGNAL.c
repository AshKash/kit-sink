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



/*
 * This component performs bit level control over the RF Monolitics radio.
 * Addtionally, it controls the amount of time per bit by using TCNT1.
 * The sample period can be set to 1/2x, 3/4x, and x. Where x is the 
 * bit transmisison period. 1/2 and 3/4 are provided to do sampling 
 * and then read at the point half way between samples.
 * 
 */



#include "tos.h"
#include "RFM_SIGNAL.h"

// #define FULLPC_DEBUG
#ifdef FULLPC
#include <Fullpc_radio.h>
#endif

#define TOS_FRAME_TYPE RFM_frame
TOS_FRAME_BEGIN(RFM_frame)
{
    char            state;
}
TOS_FRAME_END(RFM_frame);

// states:
// 0 == receive mode;
// 1 == transmit mode;
// 2 == low power mode;



#ifndef FULLPC
SIGNAL(_output_compare1a_)
{
#else
void
radioobj_interrupt()
{
    // printf("state %d, bitIn %d, first: %x\n", VAR(state), bitIn,
    // VAR(first));
    int             avilable = 0;
#endif
    char            in;
#ifndef FULLPC
    // debug: set this pin high at the start of interrupt so 
    // sample times can me measured on a scope.
    sbi(PORTD, 5);
#endif
    if (VAR(state) == 1) {
	// if we are writing, then fire the bit send event.
	TOS_SIGNAL_EVENT(RFM_TX_BIT_EVENT) ();
    } else if (VAR(state) == 0) {
#ifndef FULLPC
	// if we are reading, read in the value.
	in = READ_RFM_RXD_PIN();
#else
	// in FULLPC mode check if data is ready
	ioctl(recvfd, FIONREAD, &avilable);
	// avilable is != 0 if data is ready.
	if (avilable || recvfd == 0) {
	    // read from the socket.
	    // recvfrom(recvfd, sock_data, 1, 0,
	    // (struct sockaddr *)&recv_addr,
	    // &addr_len);
	    sock_data[0] = 0;
	    if (recvfd != 0) {
		read(recvfd, sock_data, 1);
	    }
	    // select the bit read.
	    in = sock_data[0] & 0x01;
#ifdef FULLPC_DEBUG
	    printf("got a bit\n");
	    printf("got bit %x\n", in);
#endif
#endif
	    // fire the bit arrived event and send up the value.
#ifdef FULLPC
	    TOS_SIGNAL_EVENT(RFM_RX_BIT_EVENT) (sock_data[0]);
#else
	    TOS_SIGNAL_EVENT(RFM_RX_BIT_EVENT) (in);
#endif
	}
#ifdef FULLPC
    }
#endif

}

char            TOS_COMMAND(RFM_TX_BIT) (char data) {
    // if not in the transmit mote fail.
    if (VAR(state) != 1)
	return 0;
#ifndef FULLPC
    // sent the output pin accordingly.
    if (data & 0x01) {
	SET_RFM_TXD_PIN();
    } else {
	CLR_RFM_TXD_PIN();
    }
#else
#ifdef FULLPC_DEBUG
    printf("transmitting %x\n", data & 0x01);
#endif
    // send the value down the multicast channel

#ifdef  FULLPC
    sock_data[0] = data;
#else
    sock_data[0] = data & 0x01;
#endif
    // sendto(ssend, sock_data, 1, 0,
    // (struct sockaddr *)&send_addr, sizeof(send_addr));
    if (ssend)
	write(ssend, sock_data, 1);
#endif
    return 1;
}


char            TOS_COMMAND(RFM_PWR) (char mode) {
    if (mode == 0) {
#ifndef FULLPC
	// turn off the RFM chip.
	CLR_RFM_CTL0_PIN();
	CLR_RFM_CTL1_PIN();
	// disable timer1 interupt
	outp(0x00, TCCR1B);	// scale the counter
	cbi(TIMSK, OCIE1A);
#endif
	// record the current state.
	VAR(state) = 2;
    } else if (mode == 1) {
#ifndef FULLPC
	VAR(state) = 3;
	outp(0x09, TCCR1B);	// scale the counter
	sbi(TIMSK, OCIE1A);
#endif
    }
    return 1;
}
char            TOS_COMMAND(RFM_TX_MODE) () {
    if (VAR(state) == 2)
	return 0;
#ifndef FULLPC
    // set the RFM chip to TX mode.
    SET_RFM_CTL0_PIN();
    CLR_RFM_CTL1_PIN();
#endif
#ifdef FULLPC_DEBUG
    printf("set TX mode....\n");
#endif
    // record the current state.
    VAR(state) = 1;
    return 1;
}
char            TOS_COMMAND(RFM_RX_MODE) () {
    if (VAR(state) == 2)
	return 0;
#ifndef FULLPC
    // set the RFM to RX mode.
    SET_RFM_CTL0_PIN();
    SET_RFM_CTL1_PIN();
    CLR_RFM_TXD_PIN();
#endif
#ifdef FULLPC_DEBUG
    printf("set RX mode....\n");
#endif
    // record the current state.
    VAR(state) = 0;
    return 1;
}

char            TOS_COMMAND(RFM_SET_BIT_RATE) (char level) {
#ifndef FULLPC
    if (level == 0) {
	outp(0x00, OCR1AH);	// set upper byte of comp reg.
	outp(0xc8, OCR1AL);	// set the lower byte compare
	outp(0x00, TCNT1H);	// clear current counter value
	outp(0x00, TCNT1L);	// clear current couter high byte value
    } else if (level == 1) {
	outp(0x01, OCR1AH);	// set upper byte of comp reg.
	outp(0x2c, OCR1AL);	// set the lower byte compare
    } else if (level == 2) {
	outp(0x01, OCR1AH);	// set upper byte of comp reg.
	outp(0x90, OCR1AL);	// set the lower byte compare
    }
#endif
    return 1;
}


char            TOS_COMMAND(RFM_INIT) () {
#ifndef FULLPC
    // assume RX_state.
    VAR(state) = 0;


    // set the RFM pins.
    SET_RFM_CTL0_PIN();
    SET_RFM_CTL1_PIN();
    CLR_RFM_TXD_PIN();

    cbi(TIMSK, OCIE1A);		// cear interrupts
    cbi(TIMSK, TICIE1);		// cear interrupts
    cbi(TIMSK, TOIE1);		// cear interrupts
    cbi(TIMSK, OCIE1B);		// cear interrupts
    outp(0x09, TCCR1B);		// scale the counter
    outp(0x00, TCCR1A);
    outp(0x00, OCR1AH);		// set upper byte of comp reg.
    outp(0xc8, OCR1AL);		// set the lower byte compare
    sbi(TIMSK, OCIE1A);		// enable timer1 interupt
    outp(0x00, TCNT1H);		// clear current counter value
    outp(0x00, TCNT1L);		// clear current couter high byte value
    sei();			// enable system interrupts.

#else
    printf("RFM initialized\n");
    // initialize multist socket variables.
    init_socket();
#endif
    return 1;
}
