/*
 * tab:4 NM.c - bi-directional serial-to-RF converter (wireless null
 * modem) This mote app lets you use the mote as a null modem - it
 * forwards all bytes from the UART over the radio and forwards bytes
 * received over the radio to the UART.  presumably you should be able to
 * use this for anything from a dumb terminal to a PPP/SLIP link.  there
 * is no mechanism to detect lost bytes in here - so whatever protocol is
 * to communicate over this link is responsible for its own error
 * correction.  It uses the UART on a byte level, so it should be able to 
 * interface with any regular RS232 port.  It uses AM for the RFM
 * transmission to allow for routing, avoid collissions, etc. 
 */

#include "tos.h"
#include "NM.h"

#define TOS_FRAME_TYPE NM_frame
TOS_FRAME_BEGIN(NM_frame)
{
    char            uartsend_pending;
    char            rfmsend_pending;
    char            mypartner;
    char            debug;
}
TOS_FRAME_END(NM_frame);


/*
 * NM_INIT: initialize lower components - LEDS, UART, and RFM/AM 
 */

char            TOS_COMMAND(NM_INIT) () {

    // define my "partner" for this communication - every null modem 
    // connects exactly TWO "partner" motes.  this is used as the 
    // destination address in outgoing AM packets, so it must match the 
    // address of the mote at the other end, or the packets will be lost.
    VAR(mypartner) = 0x18;

    TOS_CALL_COMMAND(NM_LEDS_INIT) ();
    TOS_CALL_COMMAND(NM_UART_INIT) ();
    TOS_CALL_COMMAND(NM_RADIO_INIT) ();

    // TOS_CALL_COMMAND(NM_LEDS_YELLOW_ON)();

    // these prevent NM from trying to send too much to either interface.
    VAR(uartsend_pending) = 0;
    VAR(rfmsend_pending) = 0;

    VAR(debug) = 0;

    printf("NM initialized\n");
    return 1;
}

char            TOS_COMMAND(NM_START) () {
    return 1;
}


// ****************************************
// Byte comes in via RFM; forward to UART

TOS_MsgPtr      TOS_EVENT(NM_RX_RADIO) (TOS_MsgPtr rfm2uartdata) {
    // TOS_CALL_COMMAND(NM_LEDS_RED_ON)();
    printf("NM received radio byte\n");
    if (VAR(uartsend_pending) == 0) {
	char            tmp;
	tmp = rfm2uartdata->data[0];	// pull value from packet
	printf("NM forwarding RFM packet to UART\n");
	if (TOS_CALL_COMMAND(NM_TX_UART) (tmp)) {
	    printf("NM UART send pending\n");
	    VAR(uartsend_pending) = 1;
	    return 1;
	}
    }
    return rfm2uartdata;
}

// **************************
// RFM completed sending byte

char            TOS_EVENT(NM_RADIO_TX_DONE) (TOS_MsgPtr msg) {
    printf("NM RFM send buffer free\n");
    VAR(rfmsend_pending) = 0;
    // TOS_CALL_COMMAND(NM_LEDS_YELLOW_OFF)();
    return 1;
}

// **********************************
// UART received byte; forward to RFM

char            TOS_EVENT(NM_RX_UART) (char uart2rfmdata, char error) {
    // TOS_CALL_COMMAND(NM_LEDS_YELLOW_ON)();

    printf("NM received UART byte\n");

    // TOS_CALL_COMMAND(NM_LEDS_GREEN_TOGGLE)();



    if (VAR(rfmsend_pending) == 0) {
	TOS_MsgPtr      u2rtmp;
	printf("NM forwarding UART byte to RFM\n");
	u2rtmp->data[0] = uart2rfmdata;
	// u2rtmp->addr = 0x18;
	u2rtmp->addr = VAR(mypartner);
	u2rtmp->group = LOCAL_GROUP;

	// return 1;

	if (TOS_CALL_COMMAND(NM_TX_RADIO)
	    (VAR(mypartner), AM_MSG(NM_RX_RADIO), u2rtmp)) {
	    // if(TOS_CALL_COMMAND(NM_TX_RADIO)(VAR(mypartner), 18,
	    // u2rtmp)){
	    printf("NM RFM send pending\n");
	    VAR(rfmsend_pending) = 1;
	    return 1;
	}
    }
    return 1;
}

// ***************************
// UART completed sending byte

char            TOS_EVENT(NM_UART_TX_DONE) (char succ) {
    printf("NM UART send buffer free\n");
    VAR(uartsend_pending) = 0;
    // TOS_CALL_COMMAND(NM_LEDS_RED_OFF)();
    return 1;
}

char            TOS_EVENT(NM_NOP) () {
    return 1;
}
