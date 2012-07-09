#include "tos.h"
#include "aps_msgs.h"
#include "SEND_COORD_DV.h"

#define TOS_FRAME_TYPE SEND_COORD_DV_frame
TOS_FRAME_BEGIN(SEND_COORD_DV_frame)
{
    char            pending;
    TOS_Msg         data;
}
TOS_FRAME_END(SEND_COORD_DV_frame);


char            TOS_COMMAND(SEND_COORD_DV_INIT) () {
    CLR_RED_LED_PIN();
    CLR_GREEN_LED_PIN();
    CLR_YELLOW_LED_PIN();
    TOS_CALL_COMMAND(SEND_COORD_DV_SUB_INIT) ();
    printf("SEND_COORD_DV initialized\n");
    SET_RED_LED_PIN();
    SET_GREEN_LED_PIN();
    SET_YELLOW_LED_PIN();

    return 1;
}


char            TOS_COMMAND(SEND_COORD_DV_OUTPUT) (int x, int y, char hops) {
    coordinates_msg *message = (coordinates_msg *) VAR(data).data;
    if (!VAR(pending)) {
	message->x = x;
	message->y = y;
	message->hops = hops;
	CLR_GREEN_LED_PIN();
	if (TOS_COMMAND(SEND_COORD_DV_SUB_SEND_MSG)
	    (TOS_BCAST_ADDR, AM_MSG(INT_READING), &VAR(data))) {

	    VAR(pending) = 1;
	    return 1;
	}
	SET_GREEN_LED_PIN();
    }
    return 0;
}

/*
 * sends the correction factor 
 */
char            TOS_COMMAND(SEND_COORD_DV_CORRECTION_OUTPUT) (int fpx,
							      int fpy,
							      int fpfactor)
{
    correction_msg *message = (correction_msg *) VAR(data).data;
    if (!VAR(pending)) {
	message->fpx = fpx;
	message->fpy = fpy;
	message->fpfactor = fpfactor;
	CLR_GREEN_LED_PIN();
	if (TOS_COMMAND(SEND_COORD_DV_SUB_SEND_MSG) (TOS_BCAST_ADDR,
						     AM_MSG
						     (CORRECTION_READING),
						     &VAR(data))) {
	    VAR(pending = 1);
	    return 1;
	}
	SET_GREEN_LED_PIN();
    }
    return 0;
}

/*
 * Sends the query reply message 
 */
char            TOS_COMMAND(SEND_COORD_DV_QUERY_REPLY) (void *reply_msg) {
    query_msg      *q_msg = (query_msg *) VAR(data).data;
    if (!VAR(pending)) {
	// have to copy the mesg as the pointer is freed by caller
	*q_msg = *(query_msg *) reply_msg;
	// this handler is not registered since, we dont want to
	// let other motes get the query replies
	// only the mote-gw gets query replies
	CLR_GREEN_LED_PIN();
	if (TOS_COMMAND(SEND_COORD_DV_SUB_SEND_MSG) (TOS_BCAST_ADDR,
						     REPLY_HANDLER,
						     &VAR(data))) {
	    VAR(pending = 1);
	    return 1;
	}
	SET_GREEN_LED_PIN();
    }
    return 0;
}

/*
 * finished sending a packet 
 */
char            TOS_EVENT(SEND_COORD_DV_SUB_MSG_SEND_DONE) (TOS_MsgPtr
							    sentBuffer) {
    if (VAR(pending) && sentBuffer == &VAR(data)) {
	VAR(pending) = 0;
	TOS_SIGNAL_EVENT(SEND_COORD_DV_COMPLETE) (1);
	SET_GREEN_LED_PIN();

	return 1;
    }
    return 0;
}


TOS_MsgPtr      TOS_MSG_EVENT(INT_READING) (TOS_MsgPtr msg) {
    coordinates_msg *message = (coordinates_msg *) msg->data;
    CLR_RED_LED_PIN();
    TOS_SIGNAL_EVENT(SEND_COORD_DV_INPUT) (message->x, message->y,
					   message->hops);
    SET_RED_LED_PIN();
    return msg;
}

/*
 * fired when the correction comes in 
 */
TOS_MsgPtr      TOS_MSG_EVENT(CORRECTION_READING) (TOS_MsgPtr msg) {
    correction_msg *message = (correction_msg *) msg->data;
    CLR_RED_LED_PIN();
    TOS_SIGNAL_EVENT(SEND_COORD_DV_CORRECTION_INPUT) (message->fpx,
						      message->fpy,
						      message->fpfactor);
    SET_RED_LED_PIN();
    return msg;

}

/*
 * fired when a query message comes 
 */
TOS_MsgPtr      TOS_MSG_EVENT(GOT_QUERY) (TOS_MsgPtr msg) {
    void           *q_message = (void *) msg->data;
    CLR_RED_LED_PIN();
    TOS_SIGNAL_EVENT(SEND_COORD_DV_QUERY_INPUT) (q_message);
    SET_RED_LED_PIN();

    return msg;

}
