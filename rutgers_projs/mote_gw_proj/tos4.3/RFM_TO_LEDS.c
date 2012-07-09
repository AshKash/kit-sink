/*									tab:4
 * RFM_TO_LEDS.c
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
 * History:   created 10/5/2000
 *
 *
 */

#include "tos.h"
#include "RFM_TO_LEDS.h"

/* Utility functions */

typedef struct{
	int val;
} int_to_led_msg;


char TOS_COMMAND(RFM_TO_LEDS_INIT)(){
  //sub pieces automatically "wired" to initialization command.
  return 1;
}

char TOS_COMMAND(RFM_TO_LEDS_START)(){
	//noting to do.
	return 1;
}

char TOS_EVENT(RFM_TO_LEDS_SUB_MSG_SEND_DONE)(TOS_MsgPtr sentBuffer){
    //this component never has sends pending.
    return 1;
}

/* Active Message handler
 * Pull out the data and send it to the output.
 */

TOS_MsgPtr TOS_MSG_EVENT(INT_READING)(TOS_MsgPtr msg){
	int_to_led_msg* message = (int_to_led_msg*)msg->data;
	TOS_CALL_COMMAND(RFM_TO_LEDS_LED_OUTPUT)(message->val);
	return msg;
}
