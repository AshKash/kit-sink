/*									tab:4
 * COUNTER.c - simple application component to display a counter on LEDS
 *
 * "Copyright (c) 2000 and The Regents of the University 
 * of California.  All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Authors:   Jason Hill
 * History:   created 1/23/2001
 *            DEC 1/30/2001: fix to use local name for output event
 *
 */

#include "tos.h"
#include "COUNTER.h"

//Frame Declaration

#define TOS_FRAME_TYPE COUNTER_frame
TOS_FRAME_BEGIN(COUNTER_frame) {
        char state;
}
TOS_FRAME_END(COUNTER_frame);

//Commands accepted

char TOS_COMMAND(COUNTER_INIT)(){
  VAR(state) = 0;
  /* initialize output component */
  return TOS_CALL_COMMAND(COUNTER_SUB_OUTPUT_INIT)();
}

char TOS_COMMAND(COUNTER_START)(){
  /* initialize clock component and start event processing */
  return TOS_CALL_COMMAND(COUNTER_SUB_CLOCK_INIT)(tick4ps);
}

//Events handled

/* Clock Event Handler:
   update LED state as 3-bit counter and set LEDs to match
 */
void TOS_EVENT(COUNTER_CLOCK_EVENT)(){
  VAR(state) ++;
  TOS_CALL_COMMAND(COUNTER_OUTPUT)(VAR(state));
}

/* Output Completion Event Handler 
   Indicate that notification was successful
*/
char TOS_EVENT(COUNTER_OUTPUT_COMPLETE)(char success){
	return 1;
}


