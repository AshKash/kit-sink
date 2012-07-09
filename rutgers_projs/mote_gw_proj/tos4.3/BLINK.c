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
 * Authors:   David Culler
 * History:   created 10/3/2000
 *
 *
 */

#include "tos.h"
#include "BLINK.h"


//Frame Declaration
#define TOS_FRAME_TYPE BLINK_frame
TOS_FRAME_BEGIN(BLINK_frame) {
        char state;
}
TOS_FRAME_END(BLINK_frame);

/* Accepts INIT Command from above*/

/* BLINK_INIT: flash all the LEDs on */
char TOS_COMMAND(BLINK_INIT)(){
  TOS_CALL_COMMAND(BLINK_LEDy_off)();
  TOS_CALL_COMMAND(BLINK_LEDr_off)();
  TOS_CALL_COMMAND(BLINK_LEDg_off)();
  TOS_CALL_COMMAND(BLINK_SUB_INIT)(255, 0x03); /* initialize clock component */
  return 1;
}
char TOS_COMMAND(BLINK_START)(){
	return 1;
}

/* Clock Event Handler:
   update LED state as 3-bit counter and set LEDs to match
 */
void TOS_EVENT(BLINK_CLOCK_EVENT)(){
  char state = VAR(state) = (VAR(state)+1) & 7;
  if (state & 1) TOS_CALL_COMMAND(BLINK_LEDy_on)();  else TOS_CALL_COMMAND(BLINK_LEDy_off)();
  if (state & 2) TOS_CALL_COMMAND(BLINK_LEDg_on)();  else TOS_CALL_COMMAND(BLINK_LEDg_off)();
  if (state & 4) TOS_CALL_COMMAND(BLINK_LEDr_on)();  else TOS_CALL_COMMAND(BLINK_LEDr_off)();
}
