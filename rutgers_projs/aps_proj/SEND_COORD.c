#include "tos.h"
#include "SEND_COORD.h"

typedef struct {
    int             x;
    int             y;
    int             dist;
} coordinates_msg;


#define TOS_FRAME_TYPE SEND_COORD_frame
TOS_FRAME_BEGIN(SEND_COORD_frame)
{
    char            pending;
    TOS_Msg         data;
}
TOS_FRAME_END(SEND_COORD_frame);


char            TOS_COMMAND(SEND_COORD_INIT) () {
    TOS_CALL_COMMAND(SEND_COORD_SUB_INIT) ();
    printf("SEND_COORD initialized\n");
    return 1;
}


char            TOS_COMMAND(SEND_COORD_OUTPUT) (int x, int y, int dist) {
    coordinates_msg *message = (coordinates_msg *) VAR(data).data;
    if (!VAR(pending)) {
	message->x = x;
	message->y = y;
	message->dist = dist;
	if (TOS_COMMAND(SEND_COORD_SUB_SEND_MSG)
	    (TOS_BCAST_ADDR, AM_MSG(INT_READING), &VAR(data))) {
	    VAR(pending) = 1;
	    return 1;
	}
    }
    return 0;
}


char            TOS_EVENT(SEND_COORD_SUB_MSG_SEND_DONE) (TOS_MsgPtr
							 sentBuffer) {
    if (VAR(pending) && sentBuffer == &VAR(data)) {
	VAR(pending) = 0;
	TOS_SIGNAL_EVENT(SEND_COORD_COMPLETE) (1);
	return 1;
    }
    return 0;
}


TOS_MsgPtr      TOS_MSG_EVENT(INT_READING) (TOS_MsgPtr msg) {
    coordinates_msg *message = (coordinates_msg *) msg->data;
    TOS_SIGNAL_EVENT(SEND_COORD_INPUT) (message->x, message->y,
					message->dist, msg->strength);
    return msg;
}
