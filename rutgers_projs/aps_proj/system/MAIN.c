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
#include "MAIN.h"


#ifdef FULLPC
#include "time.h"
struct timespec delay,
                delay1;
int             cnt = 0;
#endif

/*
 * grody stuff to set mote address into code image 
 */
const unsigned char            TOS_LOCAL_ADDRESS = MY_LOCAL_ADDRESS;

/**************************************************************
 *  Generic main routine.  Issues an init command to subordinate
 *  modules and then a start command.  These propagate down the
 *  tree as required.  The application component sits below main
 *  and above various levels of hardware support components
 *************************************************************/
#ifdef RENE
static inline void
decrease_r()
{
    SET_UD_PIN();
    CLR_POT_SELECT();
    SET_INC_PIN();
    CLR_INC_PIN();
    SET_POT_SELECT();
}

static inline void
increase_r()
{
    CLR_UD_PIN();
    CLR_POT_SELECT();
    SET_INC_PIN();
    CLR_INC_PIN();
    SET_POT_SELECT();
}


static inline void
reset_pot()
{
    unsigned char   i;
    for (i = 0; i < 200; i++) {
	decrease_r();
    }
    for (i = 0; i < 20; i++) {
	increase_r();
    }
    SET_UD_PIN();
    SET_INC_PIN();
}
#endif



// set the pot....

#ifdef FULLPC

void
usage(char *progname)
{
    fprintf(stderr, "Usage: %s node_id(decimal)\n", progname);
    fprintf(stderr, "Exiting...\n");
    exit(-1);
}
int
main(int argc, char **argv)
{
    if (argc != 2) {
	usage(argv[0]);
    }
    TOS_LOCAL_ADDRESS = atoi(argv[1]);
#else
int
main()
{
#endif




    /*
     * reset the ports, and set the directions 
     */
#ifndef FULLPC
    SET_PIN_DIRECTIONS();
#endif
#ifdef RENE
    reset_pot();
#endif

    TOS_CALL_COMMAND(MAIN_SUB_INIT) ();
    TOS_CALL_COMMAND(MAIN_SUB_START) ();
    while (1) {
	while (!TOS_schedule_task()) {
	};
#ifndef FULLPC
	sbi(MCUCR, SE);
	asm volatile    ("sleep"::);
	asm volatile    ("nop"::);
	asm volatile    ("nop"::);
#else
	/*
	 * Grody stuff to make the hw emulation work 
	 */
	delay.tv_sec = 0;
	delay.tv_nsec = 100;
	if (cnt % 1000 == 0) {
	    // THIS SLEEP LINE IS USE TO REDUCE CPU OVERHEAD AND SLOW
	    // EXECUTION
	    // IT WILL NEED TO BE REMOVED IN THE CASE OF CYGWIN
	    // nanosleep(&delay, &delay1);
	}
	if (cnt % 100000 == 0) {
	    cnt = 0;
#ifdef CLOCK_FIRE_EVENT_EVENT
	    clock_tick();
	    printf("\ntick:");
#endif
	}
	cnt++;
#ifdef ADC_DATA_READY_PORT_0_EVENT
	adc_tick();
#endif
#ifdef RFM_RX_BIT_EVENT_EVENT
	radioobj_interrupt();
#endif

	/*
	 * uncomment if running the UART simulator 
	 */
	// uarttransobj_interrupt(0);
	// uartRecobj_interrupt(0);

#endif
    }
}


char            TOS_EVENT(MAIN_SUB_SEND_DONE) (TOS_MsgPtr msg) {
    return 1;
}
