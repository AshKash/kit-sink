/*									tab:4
 * blink.c - simple application to blink the LEDs
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
 *
 *
 */

#include "tos.h"
#include "SENS_OUTPUT.h"


//Frame Declaration
#define TOS_FRAME_TYPE SENS_OUTPUT_frame
TOS_FRAME_BEGIN(SENS_OUTPUT_frame) {
        char state;
}
TOS_FRAME_END(SENS_OUTPUT_frame);

char TOS_COMMAND(SENS_OUTPUT_START)(){
  TOS_CALL_COMMAND(SENS_OUTPUT_SUB_CLOCK_INIT)(tick2ps); /* initialize clock component */
  return 1;
}


char TOS_COMMAND(SENS_OUTPUT_INIT)(){
  VAR(state) = 0;
  TOS_CALL_COMMAND(SENS_OUTPUT_SUB_OUTPUT_INIT)(); /* initialize clock component */
  TOS_CALL_COMMAND(SENS_DATA_INIT)(); /* initialize clock component */
  return 1;
}

/* Clock Event Handler:
   update LED state as 3-bit counter and set LEDs to match
 */
void TOS_EVENT(SENS_OUTPUT_CLOCK_EVENT)(){
	TOS_CALL_COMMAND(SENS_GET_DATA)();
}

/* Data ready event Handler:
   output 3-bit value 
*/
char TOS_EVENT(SENS_DATA_READY)(int data){
        TOS_CALL_COMMAND(SENS_OUTPUT_OUTPUT)((data >> 7) &0x7);
  return 1;
}

char TOS_EVENT(INT_TO_RFM_COMPLETE)(char success){
	return 1;
}
