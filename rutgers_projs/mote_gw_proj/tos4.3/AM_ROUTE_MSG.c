/*									tab:4
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
 * Authors:		Jason Hill
 *
 *
 */


//this components uses light readings as routing messages.

/* All messages are broadcast messages.  
                   [0] destionation of the message
		   [1] src id
		   [2..5] previous route
		   [6] time to live
		   [7] data
		   [8] data
  prevent cycle:  check to see if I am in the previos route

		  (ie. just route messages sent to you)
		  (if messages are not destined to you and
		  your route is not set, set it to the route.
		  Since you can set to a route to one of your children,
		  you may create a cycle.
		  Time to live is important.)
 */
#include "tos.h"
#include "AM_ROUTE_MSG.h"

extern const char TOS_LOCAL_ADDRESS;

#define TOS_FRAME_TYPE ROUTE_MSG_obj_frame
TOS_FRAME_BEGIN(ROUTE_MSG_obj_frame) {
        char route;
	char set;
	TOS_Msg route_buf;
	TOS_Msg data_buf;
	TOS_MsgPtr msg;
	char data_send_pending;
	char route_send_pending;
	int prev;
	char count;
}
TOS_FRAME_END(ROUTE_MSG_obj_frame);

char TOS_COMMAND(AM_ROUTE_MSG_START)(){
  return 1;
}

char TOS_COMMAND(AM_ROUTE_MSG_INIT)(){
    //initialize sub components
   TOS_CALL_COMMAND(ROUTE_MSG_SUB_INIT)();
   TOS_CALL_COMMAND(ROUTE_MSG_DATA_INIT)();
   VAR(msg) = &VAR(route_buf);
   VAR(data_send_pending)  = 0;
   VAR(route_send_pending) = 0;

#ifdef BASE_STATION
   //set beacon rate for route updates to be sent
 	TOS_COMMAND(ROUTE_MSG_SUB_CLOCK_INIT)(255, 0x06);
	printf("base route set to UART\n");
	//route to base station is over UART.
	VAR(route) = TOS_UART_ADDR;
	VAR(set) = 1;
	VAR(data_buf).data[0] = TOS_UART_ADDR;
	VAR(data_buf).data[1] = TOS_LOCAL_ADDRESS;
#else
	//set rate for sampling.
 	TOS_COMMAND(ROUTE_MSG_SUB_CLOCK_INIT)(255, 0x03);
	VAR(set) = 0;
	VAR(route) = 0;
	VAR(count) = 0;
#endif
   return 1;
}


TOS_MsgPtr TOS_MSG_EVENT(DATA_ROUTE_MSG)(TOS_MsgPtr msg){
  TOS_MsgPtr tmp;
  char* data = msg->data;
  
  //this handler forwards packets traveling to the base.
  TOS_CALL_COMMAND(ROUTE_MSG_LED2_TOGGLE)();

  if (data[0] == TOS_LOCAL_ADDRESS){
    if (VAR(route) != 0){
      data[5] = data[4];
      data[4] = data[3];
      data[3] = data[2];
      data[2] = data[1];
      data[1] = TOS_LOCAL_ADDRESS;
      data[0] = VAR(route);
#ifdef BASE_STATION  
      if (VAR(route_send_pending) == 0){
	TOS_CALL_COMMAND(ROUTE_MSG_SUB_SEND_MSG)(VAR(route),AM_MSG(DATA_ROUTE_MSG),msg);
	VAR(route_send_pending) = 1;
      }
      printf("routing to home %x\n", VAR(route));  
#else
      //route the packet.       
      if (VAR(route_send_pending) == 0){
	TOS_CALL_COMMAND(ROUTE_MSG_SUB_SEND_MSG)(TOS_BCAST_ADDR,AM_MSG(DATA_ROUTE_MSG),msg);
	VAR(route_send_pending) = 1;
      }
#endif

      tmp = VAR(msg);
      VAR(msg) = msg;
      return tmp;
    } // packets sent by others but not destined to me
  }
  else if(VAR(set) == 0){
    VAR(route) = data[1];
    VAR(set) = 8;
  }
  
  return msg;
}


void TOS_EVENT(AM_ROUTE_MSG_SUB_CLOCK)(){
    //clear LED3 when the clock ticks.
    TOS_CALL_COMMAND(ROUTE_MSG_LED3_OFF)();
    printf("route clock\n");
#ifdef BASE_STATION
    //if is the base, then it should send out the route update.
    if (VAR(data_send_pending) == 0){
      TOS_CALL_COMMAND(ROUTE_MSG_SUB_SEND_MSG)(TOS_BCAST_ADDR, AM_MSG(DATA_ROUTE_MSG),&VAR(data_buf));	
      VAR(data_send_pending)=1;
    }
#else
    //decrement the set var to know when a period is over.
    if(VAR(set) > 0) VAR(set) --;
    //read the value from the sensor.
    TOS_COMMAND(ROUTE_MSG_SUB_READ)();
#endif
    TOS_CALL_COMMAND(ROUTE_MSG_LED1_TOGGLE)();
    
}


char TOS_EVENT(ROUTE_MSG_SUB_DATA_READY)(int data){
    //when the data comes back from the sensor, see if the counter
    // has expired or if the  value is significantly different
    // from the previous value.
    if(VAR(route) != 0 &&
	((VAR(prev) - data) > 100 ||
	 (data - VAR(prev)) > 100 ||
	 VAR(count) > 10)){
        //if a new data packet needs to be sent, go for it.
	VAR(data_buf).data[0] = VAR(route);
	VAR(data_buf).data[1] = TOS_LOCAL_ADDRESS;
	VAR(data_buf).data[7] = data >> 8;
	VAR(data_buf).data[8] = data & 0xff;
	if (VAR(data_send_pending) == 0){
	  TOS_CALL_COMMAND(ROUTE_MSG_SUB_SEND_MSG)(TOS_BCAST_ADDR, AM_MSG(DATA_ROUTE_MSG),&VAR(data_buf));
	  VAR(data_send_pending) = 1;
	}
	VAR(count) = 0;
	//blink the LED
	TOS_CALL_COMMAND(ROUTE_MSG_LED3_ON)();
    }
    //increment the counter and store the previous reading.
    VAR(prev) = data;
    VAR(count)++;
    return 1;
}

char TOS_EVENT(ROUTE_MSG_SEND_DONE)(TOS_MsgPtr data){
  if(data == VAR(msg)) VAR(route_send_pending) = 0;
  if(data == &VAR(data_buf)) VAR(data_send_pending) = 0;
  return 1;
}


