/*									tab:4
 * INT_TO_RFM.c
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
#include "INT_TO_RFM.h"

/* Utility functions */

typedef struct{
  char val;
  char src;
} int_to_led_msg;


#define TOS_FRAME_TYPE INT_TO_RFM_frame
TOS_FRAME_BEGIN(INT_TO_RFM_frame) {
  char pending;
  TOS_Msg data; 
}
TOS_FRAME_END(INT_TO_RFM_frame);

char TOS_COMMAND(INT_TO_RFM_INIT)(){
  TOS_CALL_COMMAND(INT_TO_RFM_SUB_INIT)();
  printf("INT_TO_RFM initialized\n");
  return 1;
}

char TOS_COMMAND(INT_TO_RFM_OUTPUT)(int val){
	int_to_led_msg* message = (int_to_led_msg*)VAR(data).data;
	if (!VAR(pending)) {
	  message->val = val;
	  message->src = TOS_LOCAL_ADDRESS;
	  if (TOS_COMMAND(INT_TO_RFM_SUB_SEND_MSG)(TOS_BCAST_ADDR, AM_MSG(INT_READING), &VAR(data))) {
	    VAR(pending) = 1;
	    return 1;
	  }
	}
	return 0;
}


/* Send completion event
   Determine if this event was ours.
   If so, process it by freeing output buffer and signalling event.

*/
char TOS_EVENT(INT_TO_RFM_SUB_MSG_SEND_DONE)(TOS_MsgPtr sentBuffer){
  if (VAR(pending) && sentBuffer == &VAR(data)) {
    VAR(pending) = 0;
    TOS_SIGNAL_EVENT(INT_TO_RFM_COMPLETE)(1);
    return 1;
  }
  return 0;
}

/* Active Message handler
 */

TOS_MsgPtr TOS_MSG_EVENT(INT_READING)(TOS_MsgPtr val){
	return val;
}


