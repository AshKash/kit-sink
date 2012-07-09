#include "aps_msgs.h"
#include "APS_QUERY.h"
#include "tos.h"
#include <stdarg.h>

// the clock doesnt seem to differentiate from tick1ps and tick1000ps!
// this tries to fix it
#define TICK_SP 10		// means tick once in x seconds (or clock
				// ticks)

#define TOS_FRAME_TYPE APS_QUERY_frame
TOS_FRAME_BEGIN(APS_QUERY_frame)
{
    char            pending;
    TOS_Msg         data;
    char            current_tick;	// see TICK_SP
}
TOS_FRAME_END(APS_QUERY_frame);


char
send_query(unsigned char toq, char TOS_addr, ...)
{
    query_msg      *q_msg = (query_msg *) VAR(data).data;
    va_list         ap;

    va_start(ap, TOS_addr);

    if (VAR(pending))
	return 0;
    printf(__FUNCTION__ ": ");

    switch (toq) {
    case Q_IS_LM:
	q_msg->toq = Q_IS_LM;
	printf("Q_IS_LM\n");
	break;

    case Q_SET_LM:{		// (unsigned char value, fixpoint fpx,
	    // fixpoint fpy)
	    fixpoint       *fpptr = (fixpoint *) & q_msg->data[1];
	    unsigned char   value = (unsigned char) va_arg(ap, int);
	    fixpoint        fpx = va_arg(ap, fixpoint);
	    fixpoint        fpy = va_arg(ap, fixpoint);

	    // format message
	    q_msg->toq = Q_SET_LM;
	    q_msg->data[0] = value;
	    *fpptr++ = fpx;
	    *fpptr++ = fpy;

	    printf("Q_SET_LM: (");
	    fp_print(fpx);
	    printf(", ");
	    fp_print(fpy);
	    printf(")\n");

	    break;
	}

    case Q_GET_COORD:
	// format message
	q_msg->toq = Q_GET_COORD;
	printf("Q_GET_COORD\n");
	break;

    case Q_GET_DV_ALGO:
	break;

    case Q_SET_DV_ALGO:{	// (unsigned char algo)
	    unsigned char   algo = (unsigned char) va_arg(ap, int);

	    // format the message
	    q_msg->toq = Q_SET_DV_ALGO;
	    q_msg->data[0] = algo;
	    printf("Q_GET_DV_ALGO\n");

	    break;
	}

    case Q_DUMP_LMS:{		// (unsigned char whence)
	    unsigned char   whence = (unsigned char) va_arg(ap, int);

	    // format the message
	    q_msg->toq = Q_DUMP_LMS;
	    q_msg->data[0] = whence;
	    printf("Q_DUMP_LMS\n");
	    break;
	}

    case Q_RERUN_APS:
	// format the message
	q_msg->toq = Q_RERUN_APS;
	printf("Q_RERUN_LMS\n");
	break;

    case Q_APS_STATUS:
	// format the message
	q_msg->toq = Q_APS_STATUS;
	printf("Q_APS_STATUS\n");

	break;

    case Q_GET_CORRECTION:
	// format the message
	q_msg->toq = Q_GET_CORRECTION;
	printf("Q_GET_CORRECTION\n");

	break;

    default:
	printf("BAD Query; %d\n", q_msg->toq);
	return 0;
	break;
    }

    va_end(ap);
    // send it
    if (TOS_COMMAND(APS_QUERY_SUB_SEND_MSG) (TOS_addr,
					     QUERY_HANDLER, &VAR(data))) {
	VAR(pending = 1);
	return 1;
    }
    return 0;

}

// init
char            TOS_COMMAND(APS_QUERY_INIT) () {
    TOS_CALL_COMMAND(APS_QUERY_SUB_INIT) ();
    memset(&VAR(data), 0, sizeof(VAR(data)));
    printf("APS_QUERY initialized\n");
    return 1;
}

// start
char            TOS_COMMAND(APS_QUERY_START) () {
    return TOS_CALL_COMMAND(APS_QUERY_SUB_CLOCK_INIT) (tick1ps);
}

// clock event
void            TOS_EVENT(APS_QUERY_CLOCK_EVENT) () {
    // see TICK_SP
    // we increment the current_tick and send the message only if
    // it is above TICK_SP
    VAR(current_tick)++;
    if (VAR(current_tick) > TICK_SP) {
	VAR(current_tick) = 0;
    }

    if (VAR(current_tick) == 0) {
	int             whence;
	printf("\n DUMP_LMS: Enter whence\n");
	scanf("%d", &whence);
	send_query(Q_DUMP_LMS, TOS_BCAST_ADDR, whence);

    } else if (VAR(current_tick) == 1) {
	send_query(Q_IS_LM, TOS_BCAST_ADDR);

    } else if (VAR(current_tick) == 2) {
	send_query(Q_GET_COORD, TOS_BCAST_ADDR);

    } else if (VAR(current_tick) == 3) {
	send_query(Q_APS_STATUS, TOS_BCAST_ADDR);

    } else if (VAR(current_tick) == 4) {
	send_query(Q_GET_CORRECTION, TOS_BCAST_ADDR);

    } else if (VAR(current_tick) == 5) {
	fixpoint        x = fp_int_fix(10),
	                y = fp_int_fix(15);
	send_query(Q_SET_LM, 5, 0, x, y);
    }

}


char            TOS_EVENT(APS_QUERY_SUB_MSG_SEND_DONE) (TOS_MsgPtr
							sentBuffer) {
    if (VAR(pending) && sentBuffer == &VAR(data)) {
	VAR(pending) = 0;
	return 1;
    }
    return 0;
}


/*
 * fired when a query message comes 
 */
TOS_MsgPtr      TOS_MSG_EVENT(GOT_QUERY_REPLY) (TOS_MsgPtr msg) {
    query_msg      *q_msg = (query_msg *) msg->data;

    switch (q_msg->toq) {
    case Q_ERROR:
	printf("Error for TOQ: %d, reason: %s\n", q_msg->data[0],
	       &q_msg->data[1]);
	break;
    case REP_IS_LM:
	printf("Mote %d IS_LM: %d\n", q_msg->node_id, q_msg->data[0]);

	break;

    case REP_SET_LM:
	printf("Mote %d, SET_LM succeeded\n", q_msg->node_id);
	break;

    case REP_GET_COORD:{
	    fixpoint       *fp = (fixpoint *) q_msg->data;
	    printf("Mote %d, GET_COORD: (", q_msg->node_id);
	    fp_print(*fp++);
	    printf(", ");
	    fp_print(*fp++);
	    printf(")\n");
	    break;
	}

    case REP_GET_DV_ALGO:
	printf("REP_GET_DV_ALGO is unimplemented\n");
	break;

    case REP_SET_DV_ALGO:
	printf("REP_SET_DV_ALGO is unimplemented\n");
	break;

    case REP_DUMP_LMS:{
	    unsigned char   whence = q_msg->data[0];
	    unsigned char   no_landmarks = q_msg->data[1];
	    landmarks      *lmsptr = (landmarks *) & q_msg->data[2];

	    printf("Mote %d, DUMP_LMS: [%d, %d]\t", q_msg->node_id,
		   whence, no_landmarks);
	    while (no_landmarks--) {

		printf("(");
		fp_print(lmsptr->x);
		printf(", ");
		fp_print(lmsptr->y);
		printf(", %d)\t", lmsptr->hops);
		lmsptr++;
	    }
	    printf("\n");
	}
	break;

    case REP_RERUN_APS:
	printf("Mote %d, RERUN_APS succeeds\n", q_msg->node_id);
	break;

    case REP_APS_STATUS:
	printf("Mote %d, APS_STATUS %d\n", q_msg->node_id, q_msg->data[0]);
	break;

    case REP_GET_CORRECTION:
	printf("Mote %d, GET_CORRECTION: ", q_msg->node_id);
	fp_print(*(fixpoint *) q_msg->data);
	printf("\n");
	break;

    default:
	printf("Got UNKNOWN reply: %d\n", q_msg->toq);
	break;
    }

    return msg;

}
