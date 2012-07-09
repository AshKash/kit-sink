#include "tos.h"
#include "UART_TO_LEDS.h"

/*
 * Utility functions 
 */

#define TOS_FRAME_TYPE UART_TO_LEDS_frame
TOS_FRAME_BEGIN(UART_TO_LEDS_frame)
{
}
TOS_FRAME_END(UART_TO_LEDS_frame);


char            TOS_COMMAND(UART_TO_LEDS_INIT) () {
    TOS_CALL_COMMAND(UART_TO_LEDS_SUB_LEDS_INIT) ();
    TOS_CALL_COMMAND(UART_TO_LEDS_SUB_UART_INIT) ();
    return 1;
}
char            TOS_COMMAND(UART_TO_LEDS_START) () {
    return 1;
}
char            TOS_EVENT(UART_TO_LEDS_NOP1) (char foo) {
    return 1;
}
char            TOS_EVENT(UART_TO_LEDS_NOP2) () {
    return 1;
}

char            TOS_EVENT(UART_TO_LEDS_RX_BYTE_READY) (char data,
						       char error) {
    int             tmp = data;
    TOS_CALL_COMMAND(UART_TO_LEDS_LED_OUTPUT) (tmp);
    printf("\n");
    return 1;
}
