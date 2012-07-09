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
#include "UART.h"

#ifdef FULLPC
#include "Fullpc_uart_connect.h"
#endif

#define TOS_FRAME_TYPE UART_frame
TOS_FRAME_BEGIN(UART_frame)
{
    char            state;
}
TOS_FRAME_END(UART_frame);


char            TOS_COMMAND(UART_PWR) (char data) {
    return 0;
}


char            TOS_COMMAND(UART_INIT) () {
#ifndef FULLPC
    outp(12, UBRR);
    inp(UDR);
    outp(0xd8, UCR);
    sei();
#else
    udp_init_socket();
    printf("UART initialized\n");
#endif
    VAR(state) = 0;
    return 1;
}



#ifndef FULLPC
SIGNAL(_uart_recv_)
{
    sbi(PORTD, 6);
    if (inp(USR) & 0x80) {
	VAR(state) = 0;
	TOS_SIGNAL_EVENT(UART_RX_BYTE_READY) (inp(UDR), 0);
    }
    cbi(PORTD, 6);
}
#else
void
uartRecobj_interrupt(char bitIn)
{
    // printf("state %d, bitIn %d, first: %x\n", VAR(state), bitIn,
    // VAR(first));
    int             avail,
                    tmp;
    char            data;
    if (uart_send != 0) {
	tmp = ioctl(uart_send, FIONREAD, &avail);
	if (avail > 0) {
	    read(uart_send, &data, 1);
	    printf("UART_in_data: %x\n", data);
	    VAR(state) = 0;
	    TOS_SIGNAL_EVENT(UART_RX_BYTE_READY) (data, 0);
	}
    }
}
#endif

#ifndef FULLPC
INTERRUPT(_uart_trans_)
{
#else
void
uarttransobj_interrupt(char bitIn)
{
    // printf("state %d, bitIn %d, first: %x\n", VAR(state), bitIn,
    // VAR(first));
#endif
    VAR(state) = 0;
    TOS_SIGNAL_EVENT(UART_TX_BYTE_READY) (1);
}

char            TOS_COMMAND(UART_TX_BYTES) (char data) {

    if (VAR(state) != 0)
	return 0;
    VAR(state) = 1;
#ifdef FULLPC
    printf("UART_write_Byte_inlet %x\n", data & 0xff);
    if (uart_send != 0) {
	printf("sending byte %x over socket", data);
	write(uart_send, &data, 1);
    }
#else
    sbi(USR, TXC);
    outp(data, UDR);
#endif
    return 1;
}
