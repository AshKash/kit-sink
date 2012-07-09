/*									tab:4
 * wave.c - simple application to wave the LEDs
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
#include "WAVE.h"

/* Accepts INIT and START Commands from above*/

#define TOS_FRAME_TYPE WAVE_frame
TOS_FRAME_BEGIN(WAVE_frame) {
  char state;
  int light;
}
TOS_FRAME_END(WAVE_frame);


/* WAVE_INIT: flash all the LEDs on */
char TOS_COMMAND(WAVE_INIT)(){
  VAR(state) = 0;
  TOS_CALL_COMMAND(WAVE_LEDy_off)();
  TOS_CALL_COMMAND(WAVE_LEDr_off)();
  TOS_CALL_COMMAND(WAVE_LEDg_off)();
  TOS_COMMAND(WAVE_SUB_DATA_INIT)(); /* initialize lower components */
  TOS_COMMAND(WAVE_SUB_INIT)(255, 0x03); /* initialize lower components */
  return 1;
}

char TOS_COMMAND(WAVE_START)(){
	return 1;
}

/* Clock Event Handler:
   update LED state as 3-bit counter and set LEDs to match
 */
void TOS_EVENT(WAVE_CLOCK_EVENT)(){
  char state = VAR(state) = (VAR(state)+1) & 7;
  if (state & 1) TOS_CALL_COMMAND(WAVE_LEDy_on)();  else TOS_CALL_COMMAND(WAVE_LEDy_off)();
  TOS_COMMAND(WAVE_SUB_GET_DATA)(); /* start getting next reading */
}

char TOS_EVENT(WAVE_PHOTO_EVENT)(int newlight){
  int diff = newlight - VAR(light);
  VAR(light) = newlight;
  diff = (diff > 0) ? diff : -diff;
  if (diff > 0x50) TOS_CALL_COMMAND(WAVE_LEDr_off)(); else TOS_CALL_COMMAND(WAVE_LEDr_on)();
  return 1;
}


