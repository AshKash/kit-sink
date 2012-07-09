/*									tab:4
 * CHIRP.c - periodically emits an active message containing light reading
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
 * History:   created 10/5/2000
 *
 *
 */

#include "tos.h"
#include "CHIRP.h"

#define MAX_CHIRPS 100


//your FRAME
#define TOS_FRAME_TYPE CHIRP_frame
TOS_FRAME_BEGIN(CHIRP_frame) {
  char state;			/* Component counter state */
  TOS_Msg data; 		/* Message to be sent out */
  char send_pending;		/* Variable to store state of buffer*/
}
TOS_FRAME_END(CHIRP_frame);

/* CHIRP_INIT:  
   turn on the LEDs
   initialize lower components.
   initialize component state, including constant portion of msgs.
*/
char TOS_COMMAND(CHIRP_INIT)(){
  TOS_CALL_COMMAND(CHIRP_LEDy_on)();   
  TOS_CALL_COMMAND(CHIRP_LEDr_on)();
  TOS_CALL_COMMAND(CHIRP_LEDg_on)();       /* light LEDs */
  TOS_CALL_COMMAND(CHIRP_SUB_INIT)();       /* initialize lower components */
  VAR(state) = 0;
  VAR(data).data[0] = TOS_LOCAL_ADDRESS; //record your id in the packet.
  TOS_CALL_COMMAND(CHIRP_CLOCK_INIT)(255, 5);    /* set clock interval */
  printf("CHIRP initialized\n");
  return 1;
}


char TOS_COMMAND(CHIRP_START)(){
  TOS_CALL_COMMAND(CHIRP_GET_DATA)(); /* start data reading */
  return 1;
}

/* Clock Event Handler: 
   signaled at end of each clock interval.

 */

void TOS_EVENT(CHIRP_CLOCK_EVENT)(){
  if (VAR(state) < MAX_CHIRPS && VAR(send_pending) == 0) {
	//increment the counter
	VAR(state) ++;
	//turn on the red led while data is being read.
    	TOS_CALL_COMMAND(CHIRP_LEDr_on)();
    	TOS_CALL_COMMAND(CHIRP_GET_DATA)(); /* start data reading */
  }
}


/*  CHIRP_DATA_EVENT(data):
    handler for subsystem data event, fired when data ready.
    Put int data in a broadcast message to handler 0.
    Post msg.
 */
char TOS_EVENT(CHIRP_DATA_EVENT)(int data){
  TOS_CALL_COMMAND(CHIRP_LEDr_off)();  
  TOS_CALL_COMMAND(CHIRP_LEDg_on)();			/* Green LED while sending */
  VAR(data).data[1] = (char)(data >> 8) & 0xff;
  VAR(data).data[2] = ((char)data) & 0xff;
  if (TOS_CALL_COMMAND(CHIRP_SUB_SEND_MSG)(TOS_BCAST_ADDR,AM_MSG(CHIRP_MSG),&VAR(data))) {
    VAR(send_pending) = 1;
    return 1;
  }else {
    TOS_CALL_COMMAND(CHIRP_LEDg_off)();
    return 0;
  }
}

/*   CHIRP_SUB_MSG_SEND_DONE event handler:
     When msg is sent, shot down the radio.
*/
char TOS_EVENT(CHIRP_SUB_MSG_SEND_DONE)(TOS_MsgPtr msg){
	//check to see if the message that finished was yours.
	//if so, then clear the send_pending flag.
  if(&VAR(data) == msg){ 
	  VAR(send_pending) = 0;
	  TOS_CALL_COMMAND(CHIRP_LEDg_off)();
  }
  return 1;
}

/*   AM_msg_handler_0      handler for msg of type 0

     data: msg buffer passed
     on arrival, flash the y LED
*/
TOS_MsgPtr TOS_MSG_EVENT(CHIRP_MSG)(TOS_MsgPtr data){
  TOS_CALL_COMMAND(CHIRP_LEDy_on)();
  printf("CHIRP: %x, %x\n", data->data[0], data->data[1]);
  return data;
}


