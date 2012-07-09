/*                                                                      tab:4
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
 * Authors:             Jason Hill
 *
 *
 */

#ifndef __HARDWARE__
#define __HARDWARE__

#define RENE 1
#ifdef FULLPC

#include <time.h>
int a_holder_val;
#define inp(x...) a_holder_val = 1
#define outp(x...) a_holder_val = 1
#define sbi(x...) a_holder_val = 1
#define cbi(x...) a_holder_val = 1
#define cli() a_holder_val = 1
#define sei() a_holder_val = 1

#else //FULLPC
#include "io8535.h"
//#include "signal.h"
#include "sig-avr.h"
#include "interrupt.h"


//this macro automatically drops any printf statements if not
//compiling for a PC
#define printf(x...) ;


#endif



static inline void SET_PHOTO_CTL_PIN(){ sbi(PORTB , 4 );}
static inline void CLR_PHOTO_CTL_PIN(){ cbi(PORTB , 4 );}
static inline char READ_PHOTO_CTL_PIN(){ return 0x01 & (inp(PINB ) >> 4 );}
static inline void SET_RFM_RXD_PIN(){ sbi(PORTD , 2 );}
static inline void CLR_RFM_RXD_PIN(){ cbi(PORTD , 2 );}
static inline char READ_RFM_RXD_PIN(){ return 0x01 & (inp(PIND ) >> 2 );}
static inline void SET_RFM_CTL1_PIN(){ sbi(PORTB , 1 );}
static inline void CLR_RFM_CTL1_PIN(){ cbi(PORTB , 1 );}
static inline char READ_RFM_CTL1_PIN(){ return 0x01 & (inp(PINB ) >> 1 );}
static inline void SET_RFM_CTL0_PIN(){ sbi(PORTB , 0 );}
static inline void CLR_RFM_CTL0_PIN(){ cbi(PORTB , 0 );}
static inline char READ_RFM_CTL0_PIN(){ return 0x01 & (inp(PINB ) >> 0 );}
static inline void SET_RFM_TXD_PIN(){ sbi(PORTB , 2 );}
static inline void CLR_RFM_TXD_PIN(){ cbi(PORTB , 2 );}
static inline char READ_RFM_TXD_PIN(){ return 0x01 & (inp(PINB ) >> 2 );}




static inline void SET_RED_LED_PIN(){ sbi(PORTC , 5 );}
static inline void CLR_RED_LED_PIN(){ cbi(PORTC , 5 );}
static inline void SET_YELLOW_LED_PIN(){ sbi(PORTC , 3 );}
static inline void CLR_YELLOW_LED_PIN(){ cbi(PORTC , 3 );}
static inline void SET_GREEN_LED_PIN(){ sbi(PORTC , 4 );}
static inline void CLR_GREEN_LED_PIN(){ cbi(PORTC , 4 );}


static inline void SET_UD_PIN(){ sbi(PORTC , 4 );}
static inline void CLR_UD_PIN(){ cbi(PORTC , 4 );}
static inline void SET_INC_PIN(){ sbi(PORTC , 5 );}
static inline void CLR_INC_PIN(){ cbi(PORTC , 5 );}
static inline void SET_POT_SELECT(){ sbi(PORTC , 2 );}
static inline void CLR_POT_SELECT(){ cbi(PORTC , 2 );}
static inline void SET_MAG_POT_SELECT(){ sbi(PORTC , 1 );}
static inline void CLR_MAG_POT_SELECT(){ cbi(PORTC , 1 );}


static inline void SET_PIN_DIRECTIONS(){
         outp(0x00, DDRA);
         outp(0x00, DDRB);
         outp(0x00, DDRC);
         outp(0x00, DDRD);
        sbi(DDRC , 3 );
        sbi(DDRC , 4 );
        sbi(DDRC , 5 );
        sbi(DDRC , 2 );
        sbi(DDRC , 1 );
        sbi(DDRB , 0 );
        sbi(DDRB , 2 );
        sbi(DDRB , 1 );
        sbi(DDRB , 4 );
	SET_RED_LED_PIN();
	SET_YELLOW_LED_PIN();
	SET_GREEN_LED_PIN();
}


/* Clock scale
 * 0 - off
 * 1 - 32768 ticks/second
 * 2 - 4096 ticks/second
 * 3 - 1024 ticks/second
 * 4 - 512 ticks/second
 * 5 - 256 ticks/second
 * 6 - 128 ticks/second
 * 7 - 32 ticks/second
 */

#define tick1000ps 33,1
#define tick100ps 41,2
#define tick10ps 102,3
#define tick4096ps 1,2
#define tick2048ps 2,2
#define tick1024ps 1,3
#define tick512ps 2,3
#define tick256ps 4,3
#define tick128ps 8,3
#define tick64ps 16,3
#define tick32ps 32,3
#define tick16ps 64,3
#define tick8ps 128,3
#define tick4ps 128,4
#define tick2ps 128,5
#define tick1ps 128,6

#endif
