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


//this is a 4B-6B encoded byte-level component.

#include "tos.h"
#include "FOUR_B_RADIO_BYTE.h"
#define FULLPC_DEBUG 1

extern const char TOS_LOCAL_ADDRESS;

/* list of states:
 *
 * 0 -- wating in idle mode, searching for start symbol
 * 1 -- starting a transmission, 0 bytes encoded.
 * 2 -- 1 byte encoded
 * 3 -- 1 byte encoded, 1 byte waiting to be encoded
 * 4 -- 2 bytes encoded
 * 5 -- start symbol received waiting for first bit
 * 6 -- clocking in bits 
 *
 * 10 -- waiting to listen for idle channel
 */

//table for performing the encoding.
static const char bldTbl[16] =
{0x15,
 0x31,
 0x32,
 0x23,
 0x34,
 0x25,
 0x26,
 0x07,
 0x38,
 0x29,
 0x2A,
 0x0B,
 0x2C,
 0x0D,
 0x0E,
 0x1C};


#define TOS_FRAME_TYPE bitread_frame
TOS_FRAME_BEGIN(radio_frame) {
        char first;
        char second;
	char third;
	char fourth;
        char state;
        char count;
        char last_bit;
        char waiting;
        char wait_amount;
	unsigned int shift_reg;
}
TOS_FRAME_END(radio_frame);

TOS_TASK(radio_encode_thread)
{
    //encode byte and store it into buffer.
    VAR(fourth) = (encodeNibble(VAR(third) & 0xf));
    VAR(third) = encodeNibble((VAR(third) >> 4) & 0xf);
    //if this is the start of a transmisison, encode the start symbol.
    if(VAR(state) == 1){
	VAR(first) = 0x26; //start frame 0x00100110
	VAR(second) = 0xb5; //start frame 0x10110101
    //VAR(second) = 0x1a; //start frame 0x00011010
	VAR(count) = -2;//because this is the first byte, 0 bits have been sent.
	VAR(state) = 4;//there are now 2 bytes encoded.
    }else{
	VAR(state) = 4;//there are now 2 bytes encoded.
    }
	    
#ifdef FULLPC_DEBUG
    printf("radio_encode_thread running: %x, %x\n", VAR(third), VAR(fourth));
#endif
}

TOS_TASK(radio_decode_thread)
{
#ifdef FULLPC_DEBUG
    printf("radio_decode_thread running: %x, %x\n", VAR(third), VAR(fourth));
#endif
    //decode the byte that has been recieved.
    if (!(TOS_SIGNAL_EVENT(RADIO_BYTE_RX_BYTE_READY)(decodeNibbles(VAR(third), 
								VAR(fourth)), 0))){
	//if the event returns false, then stop receiving, go to search for the
	//start symbol at the high sampling rate.
	VAR(state) = 0;
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
    }
}		


char TOS_COMMAND(RADIO_BYTE_INIT)(){
    TOS_CALL_COMMAND(RADIO_SUB_INIT)();
    printf("Radio Byte handler initialized.\n");
    VAR(wait_amount) = 12;
    VAR(shift_reg) = 119 * TOS_LOCAL_ADDRESS;
    return 1;
}

char TOS_COMMAND(RADIO_BYTE_TX_BYTES)(char data){
#ifdef FULLPC_DEBUG
    printf("TX_bytes: state=%x, data=%x\n", VAR(state), data);
#endif
    if(VAR(state) == 0){
	//if currently in idle mode, then switch over to transmit mode
	//and set state to waiting to transmit first byte.
	VAR(third) = data;
	VAR(state) = 10;

#ifdef FULLPC 
	VAR(waiting) = 9000;
	TOS_SIGNAL_EVENT(RADIO_BYTE_RX_BIT_EVENT)(0); 
#endif
	
	return 1;
    }else if(VAR(state) == 2){
	//if in the middle of a transmission and one byte is encoded
	//go to the one byte encoded and one byte in the encode buffer.
	VAR(state) = 3;
	VAR(third) = data;
	//schedule the encode task.
	TOS_POST_TASK(radio_encode_thread);
	return 1;
    }else if(VAR(state) == 4){
	return 0;
    }
    return 0;
}

//mode 1 = active;
//mode 0 = sleep;

char TOS_COMMAND(RADIO_BYTE_PWR)(char mode){
    if(mode == 0){
	//if low power mode, tell lower components
	TOS_CALL_COMMAND(RADIO_SUB_PWR)(0);
	VAR(state) = 0xff;
    }else{
	//set the RMF component into "search for start symbol" mode.
	TOS_CALL_COMMAND(RADIO_SUB_RX_MODE)();
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
	VAR(state) = 0;
    }
    return 1;
}


char TOS_EVENT(RADIO_BYTE_TX_BIT_EVENT)(){
#ifdef FULLPC_DEBUG
    printf("radio tx bit event\n");
#endif
    //if we're not it a transmit state, return false.
    if(VAR(state) != 2 && VAR(state) != 3 && VAR(state) != 4) return 0;
    //send the next bit that we have stored.
    TOS_CALL_COMMAND(RADIO_SUB_TX_BIT)(VAR(second) & 0x01);
    //right shift the buffer.
    VAR(second) = VAR(second) >> 1;
    //increment our bytes sent count.
    VAR(count) ++;
    if(VAR(count) == 6){
	//once 6 have gone out, get ready to send out the nibble.
	VAR(second) = VAR(first);
    }else if(VAR(count) == 12){
	if(VAR(state) == 4){
	    //if another byte is ready, then shift the 
	    //data over to first and second.
	    VAR(first) = VAR(third);
	    VAR(second) = VAR(fourth);
	    VAR(count) = 0;
	    VAR(state) = 2;//now only one byte is bufferred.
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY)(1);//fire the byte transmitted event.
	}else{
	    //if there are no bytes bufferred, go back to idle.
	    VAR(state) = 0;
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY)(1);
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_DONE)();
	    TOS_CALL_COMMAND(RADIO_SUB_RX_MODE)();
	    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
	}
    }
    return 1;
}	
		
char TOS_EVENT(RADIO_BYTE_RX_BIT_EVENT)(char data){
char bit;
#ifdef FULLPC
    //because the FULLPC version doesn't do 2x sampling, we fake it out with
    //this.
    VAR(last_bit) = data;
#endif
    if(VAR(state) == 0){
	//if we are in the idle state, the check if the bit read
	//matches the last bit.
	if(VAR(last_bit) != data){
	    VAR(last_bit) = data;
	    return 1;
	}
	//if so, set last bit to invalid,
	VAR(last_bit) = 0xff;
	//right shift previously read data.
	VAR(second) >>= 1;
	//mask out upper bit
	VAR(second) &= 0x7f; 
	//if lowest bit of first is one, store it in second
	if(VAR(first) & 0x1) VAR(second) =  VAR(second) | 0x80;
	//don't forget that the start symbol is only 9 bits long. 
	VAR(first) = data & 0x1;
#ifdef FULLPC_DEBUG
	//printf("checking for start symbol: %x, %x\n", VAR(first) & 0xff,
	 //      VAR(second) & 0xff);
#endif
	//if you now have the start symbol, go to the waiting for first bit state.
	if(VAR(first) == (char)0x1 && VAR(second) == (char)0x35){
	    VAR(state) = 5;
	    VAR(count) = 0;
	    //set bit rate so next sample falls in middle of next
	    //transmitted bit.
	    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(1);
	}
	
    }else if(VAR(state) == 5){
	//just read first bit.
	//set bit rate to match TX rate.
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(2);
	VAR(state) = 6;
	VAR(count) = 1;
	//store it.
	if(data){
	    VAR(first) = 0x20;
	}else{
	    VAR(first) = 0;
	}
    }else if(VAR(state) == 6){
	//clock in bit.
	VAR(count)++;
	VAR(first) >>= 1;
	VAR(first) &= 0x1f;
	if(data){
	    VAR(first) |= 0x20;
	}
	if(VAR(count) == 6){
	    VAR(fourth) = VAR(first);
	}else if(VAR(count) == 12){
#ifdef FULLPC_DEBUG
	printf("entire byte received: %x, %x\n", VAR(third), VAR(fourth));
#endif
	    VAR(count) = 0;
	    //sore the encoded data into a buffer.
	    VAR(third) = VAR(first);
	    VAR(state) = 5;
	    //scheduled the decode task.
	    TOS_POST_TASK(radio_decode_thread);
	}
    }else if(VAR(state) == 10){
	//waiting for channle to be idle.
	if(data){
	    //if we just read activity, then reset the waiting counter.
	   VAR(waiting) = 0;
        }else{
	   if (++VAR(waiting) == 11){
	       bit = (VAR(shift_reg) & 0x2) >> 1;
	       bit ^= ((VAR(shift_reg) & 0x4000) >> 14);
	       bit ^= ((VAR(shift_reg) & 0x8000) >> 15);
	       VAR(shift_reg) >>=1;
	       if (bit & 0x1)
		   VAR(shift_reg) |= 0x8000;
	       VAR(wait_amount) = (VAR(shift_reg) & 0x3f)+12;
	   }
	   
	   //if we've not heard anything for 8 samples then...
	   if(VAR(waiting) > VAR(wait_amount)){
	       //go to the transmitting state.
	       VAR(state) = 1;
	       VAR(waiting) = 0;
	       //schedule task to start transfer, set TX_mode, and set bit rate
	       TOS_CALL_COMMAND(RADIO_SUB_TX_MODE)();
	       TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(2);
	       TOS_POST_TASK(radio_encode_thread);
	   }	
	}	
    }
    return 1;
}



char encodeNibble(char in){
    //use table to encode data.
    return bldTbl[(int)in];

}

char decodeNibbles(char first, char second){
    //generic function for decoding data.
    char out;
    if (first == 0x15) first = 0;
    else if(first == 0x1c) first = 0xf;
    if (second == 0x15) second = 0;
    else if(second == 0x1c) second = 0xf;
    out = first << 4;
    out |= second & 0xf;
    return out;
}
