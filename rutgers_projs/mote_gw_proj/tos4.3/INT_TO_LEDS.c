/*	LED output component						tab:4
 * 
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
 * History:   created 1/29/2001
 *
 *
 */

#include "tos.h"
#include "INT_TO_LEDS.h"

/* INT_TO_LEDS_INIT: flash all the LEDs on */
char TOS_COMMAND(INT_TO_LEDS_INIT)(){
  TOS_CALL_COMMAND(INT_TO_LEDS_SUB_INIT)(); /* initialize LED component */
  TOS_CALL_COMMAND(INT_TO_LEDS_LEDy_off)(); /* turn all off */
  TOS_CALL_COMMAND(INT_TO_LEDS_LEDr_off)();
  TOS_CALL_COMMAND(INT_TO_LEDS_LEDg_off)();
  return 1;
}

/* INT_TO_LEDS_OUTPUT command
   set LEDs to match 3-bit value
 */
char TOS_COMMAND(INT_TO_LEDS_OUTPUT)(int state){
  if (state & 1) TOS_CALL_COMMAND(INT_TO_LEDS_LEDy_on)();  else TOS_CALL_COMMAND(INT_TO_LEDS_LEDy_off)();
  if (state & 2) TOS_CALL_COMMAND(INT_TO_LEDS_LEDg_on)();  else TOS_CALL_COMMAND(INT_TO_LEDS_LEDg_off)();
  if (state & 4) TOS_CALL_COMMAND(INT_TO_LEDS_LEDr_on)();  else TOS_CALL_COMMAND(INT_TO_LEDS_LEDr_off)();
  return 1;			/* indicate success */
}



