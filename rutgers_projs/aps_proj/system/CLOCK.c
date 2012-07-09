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
#include "CLOCK.h"


char            TOS_COMMAND(CLOCK_INIT) (char interval, char scale) {

    printf("Clock initialized\n");
    scale &= 0x7;
    scale |= 0x8;
    cbi(TIMSK, TOIE2);
    cbi(TIMSK, OCIE2);		// Disable TC0 interrupt
    sbi(ASSR, AS2);		// set Timer/Counter0 to be asynchronous
				// from the CPU clock
    // with a second external clock(32,768kHz)driving it.
    outp(scale, TCCR2);		// prescale the timer to be clock source / 
				// 128 to make it
    outp(0, TCNT2);
    outp(interval, OCR2);
    sbi(TIMSK, OCIE2);
    sei();
    return 1;
}


#ifdef FULLPC
void
clock_tick()
{
#else
INTERRUPT(_output_compare2_)
{
#endif
    TOS_SIGNAL_EVENT(CLOCK_FIRE_EVENT) ();
}
