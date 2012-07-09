/*
 * MOTE_GW.c - Responsible for sending packets from serial to rfm and vice versa
 * Derived from GENERIC_BASE.c
 * 
 * Authors:   Ashwin Kashyap
 * History:   created 06/27/2001
 *
 */

#include "tos.h"
#include "MOTE_GW.h"

/* Utility functions */

#define TOS_FRAME_TYPE MOTE_GW_frame
TOS_FRAME_BEGIN(MOTE_GW_frame) {
  TOS_Msg data;
  TOS_Msg rfm_data; 
  TOS_MsgPtr msg;
  TOS_MsgPtr rfm_msg;
  char send_pending;
  char rfm_send_pending;
}
TOS_FRAME_END(MOTE_GW_frame);


/* MOTE_GW_INIT:  
   initialize lower components.
   initialize component state, including constant portion of msgs.
*/
char TOS_COMMAND(MOTE_GW_INIT)(){
  CLR_GREEN_LED_PIN();
  TOS_CALL_COMMAND(MOTE_GW_SUB_INIT)();       /* initialize lower components */
  TOS_CALL_COMMAND(MOTE_GW_SUB_UART_INIT)();       /* initialize lower components */
  VAR(msg) = &VAR(data);
  VAR(rfm_msg) = &VAR(rfm_data);
  VAR(send_pending) = 0;
  VAR(rfm_send_pending) = 0;

  printf("MOTE_GW initialized\n");
  SET_GREEN_LED_PIN();

  /* Tmp code !!!!!!!!!!!!!!!!!! */
  CLR_RED_LED_PIN();
  TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES)(FLOW_CONT);
  
  return 1;
}
char TOS_COMMAND(MOTE_GW_START)(){
	return 1;
}

/* Called when UART finishes trasnsmission of 1 byte */
char TOS_EVENT(MOTE_GW_SUB_UART_TX_DONE)(void)
{
  /* Tx another byte to the uart */
  SET_RED_LED_PIN();
  if (TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES)(FLOW_CONT) == 0)
    CLR_YELLOW_LED_PIN();

}

/* The RFM got a packet */
TOS_MsgPtr TOS_EVENT(MOTE_GW_RX_PACKET)(TOS_MsgPtr data){
	TOS_MsgPtr tmp = data;
	printf("MOTE_GW received packet\n");
	if(VAR(send_pending) == 0 && data->group == LOCAL_GROUP){
		tmp = VAR(msg);
		VAR(msg) = data;
		printf("MOTE_GW forwarding packet\n");
		if(TOS_COMMAND(MOTE_GW_SUB_UART_TX_PACKET)(data)){
			printf("MOTE_GW send pending\n");
			CLR_RED_LED_PIN();
			VAR(send_pending)  = 1;
		}
	}
	return tmp;
}

/* RFM finished sending the packet */
char TOS_EVENT(MOTE_GW_TX_PACKET_DONE)(TOS_MsgPtr data){

	// This event gets triggered when the RFM finishes
	// sending out a pkt
	if (VAR(rfm_msg) == data) {
		printf("MOTE_GW rfm send buffer free\n");
		VAR(rfm_send_pending) = 0;
		SET_GREEN_LED_PIN();

		// Flow control 
		if (TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES)(FLOW_CONT) == 0)
		  CLR_YELLOW_LED_PIN();

	}
	return 1;
}

/* The UART got a packet */
TOS_MsgPtr TOS_EVENT(MOTE_GW_SUB_UART_RX_PACKET)(TOS_MsgPtr data){
	TOS_MsgPtr tmp = data;

	// Got a pkt from the UART, now xmit over the RFM
	// Make sure there are no sends that are pending over the
	// RFM. The packet is dropped and a message is sent
	// back to the UART in such a situation

	printf("MOTE_GW: Got pkt from UART\n");

	if (VAR(rfm_send_pending) == 0 && data->group == LOCAL_GROUP) {	
		// save some stuff 
		tmp = VAR(rfm_msg);
		VAR(rfm_msg) = data;
		SET_YELLOW_LED_PIN();
		printf("MOTE_GW fwd pkt from UART to RFM\n");

		// processing pkt goes here (or spawn off a task to do it ?)

		// Send flow control to serial
		/*
		  if (TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES)(FLOW_STOP) == 0) 
		  goto ERROR_RFM_TX;
		*/

		// finally transmit the data
		if (TOS_CALL_COMMAND(PACKET_TX_PACKET)(data)) {
			printf("MOTE_GW RFM send pending\n");
			CLR_GREEN_LED_PIN();
			VAR(rfm_send_pending) = 1;

		} else {
			goto ERROR_RFM_TX;
		}


	} else {
		// Some sort of error has occured
ERROR_RFM_TX:
/*
		tmp = data;
		// The error pkt must be filled here
		tmp->addr = 0x7e;
		tmp->group = LOCAL_GROUP;
		tmp->type = 0xff;	// The flow control error handler


		// Failed
		printf("MOTE_GW: Error while sending to RFM\n");
*/
		CLR_YELLOW_LED_PIN();
	        TOS_CALL_COMMAND(UART_PACKET_SUB_TX_BYTES)(FLOW_STOP);
/*
		// Try to do flow control
		// send a special packet back to the uart
		// saying sender must slow down. failure is not checked
		TOS_CALL_COMMAND(MOTE_GW_SUB_UART_TX_PACKET)(tmp);
	*/
	}

	return tmp;
}

/* The UART finished sending out a packet */
char TOS_EVENT(MOTE_GW_SUB_UART_TX_PACKET_DONE)(TOS_MsgPtr data){
	if(VAR(msg) == data) {
		printf("MOTE_GW send buffer free\n");
		SET_RED_LED_PIN();
		VAR(send_pending) = 0;
	}
	return 1;
}

