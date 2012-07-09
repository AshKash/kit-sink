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

#include "tos.h"
//#define FULLPC_DEBUG 1
#include "RADIO_BYTE.h"


#define TOS_FRAME_TYPE bitread_frame
TOS_FRAME_BEGIN(radio_frame) {
        char first;
        char second;
	char third;
	char fourth;
        char state;
        char count;
        char last_bit;
      
}
TOS_FRAME_END(radio_frame);

TOS_TASK(radio_encode_thread)
{

	VAR(fourth) = encodeNibble(VAR(third) & 0xf);
	VAR(third) = encodeNibble((VAR(third) >> 4) & 0xf);
	if(VAR(state) == 1){
	  VAR(first) = 0x99; //start frame
	  VAR(second) = 0x9a; //start frame
	  VAR(count) = 0;
	  VAR(state) = 4;
	}else{
	    VAR(state) = 4;
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

    if(!(TOS_SIGNAL_EVENT(RADIO_BYTE_RX_BYTE_READY)(decodeNibbles(VAR(third), VAR(fourth)), 0))){
	VAR(state) = 0;
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
    }
}		


char TOS_COMMAND(RADIO_BYTE_INIT)(){
    TOS_CALL_COMMAND(RADIO_SUB_INIT)();
    printf("Radio Byte handler initialized.\n");
    return 1;
}

char TOS_COMMAND(RADIO_BYTE_TX_BYTES)(char data){
#ifdef FULLPC_DEBUG
	printf("TX_bytes: state=%x, data=%x\n", VAR(state), data);
#endif
    if(VAR(state) == 0){
	VAR(third) = data;
	VAR(state) = 1;
	//schedule task to start transfer
	TOS_CALL_COMMAND(RADIO_SUB_TX_MODE)();
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(2);
	TOS_POST_TASK(radio_encode_thread);
	return 1;
    }else if(VAR(state) == 2){
	VAR(state) = 3;
	VAR(third) = data;
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
	TOS_CALL_COMMAND(RADIO_SUB_PWR)(0);
	VAR(state) = 0xff;
    }else{
	TOS_CALL_COMMAND(RADIO_SUB_RX_MODE)();
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
	VAR(state) = 0;
    }
    return 1;
}


char  TOS_EVENT (RADIO_BYTE_TX_BIT_EVENT)(){
#ifdef FULLPC_DEBUG
    printf("radio tx bit event\n");
#endif
	if(VAR(state) != 2 && VAR(state) != 3 && VAR(state) != 4) return 0;
	TOS_CALL_COMMAND(RADIO_SUB_TX_BIT)(VAR(second) & 0x01);
	VAR(second) = VAR(second) >> 1;
	VAR(count) ++;
	if(VAR(count) == 8){
		VAR(second) = VAR(first);
	}else if(VAR(count) == 16){
	        if(VAR(state) == 4){
			VAR(first) = VAR(third);
			VAR(second) = VAR(fourth);
			VAR(count) = 0;
			VAR(state) = 2;
			TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY)(1);
		}else{
			VAR(state) = 0;
			TOS_CALL_COMMAND(RADIO_SUB_RX_MODE)();
			TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY)(1);
			TOS_SIGNAL_EVENT(RADIO_BYTE_TX_DONE)();
			TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
		}
	}
	return 1;
}	
		
char TOS_EVENT ( RADIO_BYTE_RX_BIT_EVENT)(char data){
#ifdef FULLPC
    VAR(last_bit) = data;
#endif
    if(VAR(state) == 0){
	if(VAR(last_bit) != data){
	    VAR(last_bit) = data;
	    return 1;
	}
	VAR(last_bit) = 0xff;
	VAR(second) >>= 1;
	VAR(second) &= 0x7f;
	if(VAR(first) & 0x1) VAR(second) =  VAR(second) | 0x80;
	VAR(first) >>= 1;
	VAR(first) &= 0x7f;
	if(data){
	    VAR(first) = VAR(first) | 0x80;
	}
#ifdef FULLPC_DEBUG
	//printf("checking for start symbol: %x, %x\n", VAR(first) & 0xff,
	 //      VAR(second) & 0xff);
#endif
	if(VAR(first) == (char)0x99 && VAR(second) == (char)0x9a){
	    VAR(state) = 5;
	    VAR(count) = 0;
	    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(1);
	}
	
    }else if(VAR(state) == 5){
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(2);
	VAR(state) = 6;
	VAR(count) = 1;
	if(data){
	    VAR(first) = 0x80;
	}else{
	    VAR(first) = 0;
	}
    }else if(VAR(state) == 6){
	VAR(count)++;
	VAR(first) >>= 1;
	VAR(first) &= 0x7f;
	if(data){
	    VAR(first) |= 0x80;
	}
	if(VAR(count) == 8){
	    VAR(fourth) = VAR(first);
	}else if(VAR(count) == 16){
#ifdef FULLPC_DEBUG
	printf("entire byte received: %x, %x\n", VAR(third), VAR(fourth));
#endif
	    VAR(count) = 0;
	    VAR(third) = VAR(first);
	    VAR(state) = 5;
	    TOS_POST_TASK(radio_decode_thread);
	}
    }
	return 1;
}



char encodeNibble(char in){
    char out = (in & 0x8) << 4;
    out |= (in & 0x4) << 3;
    out |= (in & 0x2) << 2;
    out |= (in & 0x1) << 1;
    out |= (~out & 0xaa) >> 1;
    return out;
}

char decodeNibbles(char first, char second){
        char out = first & 0x80;
        out |= (first << 1) & 0x40;
        out |= (first << 2) & 0x20;
        out |= (first << 3) & 0x10;
        out |= second >> 4 & 0x08;
        out |= second >> 3 & 0x04;
        out |= second >> 2 & 0x02;
        out |= second >> 1 & 0x01;
        return out;
}
