#include "tos.h"
#include "LOGGER_TEST.h"

struct log_rec {
    short           time;
    short           val;
};

typedef struct {
    struct log_rec  records[8];
} log_entry;


/*
 * Utility functions 
 */
#define TOS_FRAME_TYPE LOGGER_TEST_frame
TOS_FRAME_BEGIN(LOGGER_TEST_frame)
{
    int             time;
    char            count;
    TOS_Msg         msg;
    TOS_Msg         read_msg;
    char            msg_pending;
    char            read_send_pending;
    log_entry      *entries;
}
TOS_FRAME_END(LOGGER_TEST_frame);

char            TOS_COMMAND(LOGGER_TEST_INIT) () {
    VAR(time) = 0;
    VAR(msg_pending) = 0;
    VAR(read_send_pending) = 0;
    TOS_CALL_COMMAND(LOGGER_TEST_SUB_INIT) ();
    TOS_CALL_COMMAND(COMM_INIT) ();
    TOS_CALL_COMMAND(LOGGER_CLOCK_INIT) (255, 2);
    TOS_CALL_COMMAND(LOGGER_ADC_INIT) ();
    VAR(entries) = (log_entry *) VAR(msg).data;
    printf("LOGGER_TEST initialized\n");
    return 1;
}

char            TOS_COMMAND(LOGGER_TEST_START) () {
    return 1;
}

void            TOS_EVENT(LOGGER_TEST_CLOCK_EVENT) () {
    VAR(time)++;
    printf("getting data\n");
    TOS_CALL_COMMAND(YELLOW_LED_TOGGLE) ();
    TOS_CALL_COMMAND(LOGGER_ADC_GET_DATA) (1);	/* start data reading */
}

char            TOS_EVENT(LOGGER_TEST_WRITE_LOG_DONE) (char success) {
    printf("LOG_WRITE_DONE\n");
    if (VAR(msg_pending) == 0) {
	TOS_CALL_COMMAND(GREEN_LED_TOGGLE) ();
	VAR(msg_pending) =
	    TOS_CALL_COMMAND(COMM_SEND_MSG) (TOS_UART_ADDR, 0x06,
					     &VAR(msg));
    }
    return 1;
}
TOS_MsgPtr      TOS_EVENT(LOGGER_TEST_READ_MSG) (TOS_MsgPtr msg) {
    char           *data = msg->data;
    int             log_line = data[1] & 0xff;
    log_line |= data[0] << 8;
    printf("LOG_READ_START \n");
    TOS_CALL_COMMAND(GREEN_LED_TOGGLE) ();
    TOS_CALL_COMMAND(LOGGER_TEST_READ_LOG) (log_line, VAR(read_msg).data);
    return msg;
}
char            TOS_EVENT(LOGGER_TEST_MSG_SENT) (TOS_MsgPtr msg) {
    if (VAR(msg_pending) == 1 && ((msg == &VAR(msg)))) {
	VAR(msg_pending) = 0;
	printf("data buffer free\n");
    }
    if (VAR(read_send_pending) == 1 && ((msg == &VAR(read_msg)))) {
	VAR(read_send_pending) = 0;
	printf("read buffer free\n");
    }
    return 0;
}

char            TOS_EVENT(LOGGER_TEST_READ_LOG_DONE) (char *data,
						      char success) {
    printf("LOG_READ_DONE\n");
    TOS_CALL_COMMAND(COMM_SEND_MSG) (TOS_UART_ADDR, 0x06, &VAR(read_msg));
    return 1;
}

char            TOS_EVENT(LOGGER_DATA_READY) (int data) {
    printf("got logger data\n");
    TOS_CALL_COMMAND(RED_LED_TOGGLE) ();
    VAR(entries)->records[(int) VAR(count)].val = data;
    VAR(entries)->records[(int) VAR(count)].time = VAR(time);
    VAR(count)++;
    if (VAR(count) > 7) {
	TOS_CALL_COMMAND(LOGGER_TEST_WRITE_LOG) (VAR(msg).data);
	VAR(count) = 0;
    }
    return 1;
}
