/*
 *
 * Copyright (c) 2001 Rutgers University and Richard P. Martin.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. All advertising materials mentioning features or use of this
 *       software must display the following acknowledgment:
 *           This product includes software developed by 
 *           Rutgers University and its contributors.
 *
 *    4. Neither the name of the University nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 * 
 * IN NO EVENT SHALL RUTGERS UNIVERSITY BE LIABLE TO ANY PARTY FOR DIRECT, 
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF RUTGERS
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  
 * RUTGERS UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND RUTGERS UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *
 * Author:                      Richard P. Martin 
 * Version:                     $Id: UART_2_LEDS.c,v 1.2 2002/04/08 16:51:46 ashwink Exp $
 * Creation Date:               Tue Jul 10 18:02:58 2001
 * Filename:                    UART_2_LEDS.c
 * History:
 * $Log: UART_2_LEDS.c,v $
 * Revision 1.2  2002/04/08 16:51:46  ashwink
 *
 * Dont panic!
 * Did a general cleanup to remove characters after #endif
 *
 * Revision 1.1.1.1  2002/03/08 23:52:37  ashwink
 *
 * APS Code includes:
 *      mote code
 *      mote-gw
 *      tcl/tk demo files
 *
 * Revision 1.1.1.1  2001/10/04 04:12:06  rmartin
 * hopefully this is correct
 *
 * Revision 1.1  2001/07/10 14:01:52  rmartin
 * small program to display lower 3 bits of the uart on the leds
 *
 */

#include "tos.h"
#include "UART_2_LEDS.h"

/*
 * ------- no frame declaration --------
 */

/*
 * ------- commands accepted --------- 
 */
char            TOS_COMMAND(UART_2_LEDS_INIT) () {
    /*
     * initialize output component 
     */

    if (TOS_CALL_COMMAND(UART_2_LEDS_LEDS_INIT) () == 0)
	return 0;

    return TOS_CALL_COMMAND(UART_2_LEDS_UART_INIT) ();

}

char            TOS_COMMAND(UART_2_LEDS_START) () {
    /*
     * don't do anything for now 
     */
    return 1;
}

/*
 * -------events handled---------- 
 */
/*
 * recieve a packet from the UART 
 */
char            TOS_EVENT(UART_2_LEDS_RX_BYTE_READY) (char data,
						      char error) {

#ifdef FULLPC_DEBUG
    printf("UART PACKET: byte arrived: %x, STATE: %d, COUNT: %d\n",
	   data, VAR(state), VAR(count));
#endif

    if (error) {
	return 0;
    } else {

	return TOS_CALL_COMMAND(UART_2_LEDS_OUTPUT) (data);
    }
}

/*
 * don't bother with TX side, stubs for now 
 */
char            TOS_EVENT(UART_2_LEDS_TX_BYTE_READY) (char success) {
    return 1;
}

char            TOS_EVENT(UART_PACKET_BYTE_TX_DONE) () {
    return 1;
}
