/*									tab:4
 * MAGS.c - periodically emits an active message containing light reading
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
#include "MAGS.h"

/* Utility functions */

#define DATA_PORT 5

struct adc_packet{
	int count;
	int data[DATA_LENGTH/sizeof(int) - 1];
};


#define TOS_FRAME_TYPE MAGS_frame
TOS_FRAME_BEGIN(MAGS_frame) {
  volatile char led_on;			/* counter state */
  volatile unsigned char pot_wait;	/* counter state */
  volatile char state;			/* Component counter state */
  TOS_Msg msg;
  volatile char send_pending;
  volatile int first;		//storage to hold various filters.
  volatile int second;		//storage to hold various filters.
  volatile int diff;		//storage to hold various filters.
  volatile int reading;
  
}
TOS_FRAME_END(MAGS_frame);


/* MAGS_INIT:  
   flash the LEDs
   initialize lower components.
   initialize component state, including constant portion of msgs.
*/
char TOS_COMMAND(MAGS_INIT)(){
  struct adc_packet* pack = (struct adc_packet*)(VAR(msg).data);
  SET_MAG_POT_SELECT();
  TOS_CALL_COMMAND(MAGS_LEDy_off)();   
  TOS_CALL_COMMAND(MAGS_LEDr_off)();
  TOS_CALL_COMMAND(MAGS_LEDg_off)();       /* light LEDs */
  TOS_CALL_COMMAND(MAGS_SUB_INIT)();       /* initialize lower components */
  TOS_CALL_COMMAND(MAGS_CLOCK_INIT)(40, 1);    /* set clock interval */
  VAR(state) = 0;
  VAR(send_pending) = 0;
  pack->count = 0; 
  printf("MAGS initialized\n");
  return 1;
}

/* MAGS_START
   start data reading.
*/
char TOS_COMMAND(MAGS_START)(){
  TOS_CALL_COMMAND(MAGS_GET_DATA)(DATA_PORT); /* start data reading */
  return 1;
}


//utility functions to deal with variable potentometer.
static inline void decrease_r() {
    SET_INC_PIN();
    CLR_UD_PIN();
    CLR_MAG_POT_SELECT();
    SET_UD_PIN();
    CLR_UD_PIN();
    SET_MAG_POT_SELECT();
}

static inline void increase_r() {
    CLR_INC_PIN();
    CLR_UD_PIN();
    CLR_MAG_POT_SELECT();
    SET_UD_PIN();
    CLR_UD_PIN();
    SET_MAG_POT_SELECT();
}

//data is ready.

TOS_TASK(FILTER_DATA){
  int tmp;
  VAR(first) = VAR(first) - (VAR(first) >> 5);
  VAR(first) += VAR(reading);
  VAR(second) = VAR(second) - (VAR(second) >> 5);
  VAR(second) += VAR(first) >> 5;
  VAR(diff) = VAR(diff) - (VAR(diff) >> 5);
  tmp = VAR(first) - VAR(second);
  if(tmp < 0) tmp = -tmp;
  VAR(diff) += tmp;
  if((VAR(diff) >> 5) > 85){
  	TOS_CALL_COMMAND(MAGS_LEDg_on)();
	VAR(led_on) = 255;
  }
	
	
}

char TOS_EVENT(MAGS_DATA_EVENT)(int data){
  struct adc_packet* pack = (struct adc_packet*)(VAR(msg).data);
  printf("data_event\n");
  VAR(reading) = data;
  TOS_POST_TASK(FILTER_DATA);
  if(VAR(send_pending) == 0){
	pack->count ++; 
	pack->data[(int)VAR(state)] = data;
  	VAR(state) ++;
  } 
	
  if(VAR(pot_wait) == 0){
  		TOS_CALL_COMMAND(MAGS_LEDy_off)();
  		TOS_CALL_COMMAND(MAGS_LEDr_off)();
  	if(data > 0x200){
		decrease_r();
		//this is a counter so that we can only reset the POT once
		//every 255 samples.
  		TOS_CALL_COMMAND(MAGS_LEDy_on)();
		VAR(pot_wait) = 255;
	}
  	else if(data < 0x100){
		increase_r();
		//this is a counter so that we can only reset the POT once
		//every 255 samples.
		VAR(pot_wait) = 255;
  		TOS_CALL_COMMAND(MAGS_LEDr_on)();
		
		
	}
  }

  if(VAR(state) == sizeof(pack->data)/sizeof(int)){
  	pack->data[(int)VAR(state)] = VAR(diff);
	VAR(state) = 0;
  	if (TOS_CALL_COMMAND(MAGS_SUB_SEND_MSG)(TOS_UART_ADDR,AM_MSG(mags_msg),&VAR(msg))) {
		VAR(send_pending) = 1;
    		return 1;
	}else {
    		return 0;
  	}
  }
  return 1;
}


/*   

     data: msg buffer passed
     on arrival, flash the y LED
*/
TOS_MsgPtr TOS_MSG_EVENT(mags_msg)(TOS_MsgPtr msg){
  printf("MAGS: %x, %x\n", msg->data[0], msg->data[1]);
  return msg;
}
/*   MAGS_SUB_MSG_SEND_DONE event handler:
     When msg is sent, shot down the radio.
*/
char TOS_EVENT(MAGS_SUB_MSG_SEND_DONE)(TOS_MsgPtr msg){
  if(msg == &VAR(msg)){
	printf("MAGS send buffer free\n");
	VAR(send_pending) = 0;
  }
  return 1;
}



/* Clock Event Handler: 
   signaled at end of each clock interval.

 */
void TOS_EVENT(MAGS_CLOCK_EVENT)(){
  TOS_CALL_COMMAND(MAGS_GET_DATA)(DATA_PORT); /* start data reading */

  //count down the LED_on variable.  when it hits 0, turn off the led.
  if(VAR(led_on == 1)) TOS_CALL_COMMAND(MAGS_LEDg_off)();
  if(VAR(led_on) != 0) VAR(led_on) --;

  //count done the pot_wait variable
  if(VAR(pot_wait) != 0) VAR(pot_wait) --;
}

