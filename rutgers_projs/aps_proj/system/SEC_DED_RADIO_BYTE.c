/*
 * tab:4 "Copyright (c) 2000 and The Regents of the University of
 * California.  All rights reserved. Permission to use, copy, modify, and 
 * distribute this software and its documentation for any purpose, without 
 * fee, and without written agreement is hereby granted, provided that the 
 * above copyright notice and the following two paragraphs appear in all
 * copies of this software. IN NO EVENT SHALL THE UNIVERSITY OF
 * CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS
 * SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE UNIVERSITY OF
 * CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 * Authors: Jason Hill 
 */




#include "tos.h"
#include "SEC_DED_RADIO_BYTE.h"

/*
 * list of states: 0 -- wating in idle mode, searching for start symbol 1 
 * -- starting a transmission, 0 bytes encoded. 2 -- 1 byte encoded 3 -- 1 
 * byte encoded, 1 byte waiting to be encoded 4 -- 2 bytes encoded 5 --
 * start symbol received waiting for first bit 6 -- clocking in bits 10
 * -- waiting to listen for idle channel 
 */

extern const unsigned char TOS_LOCAL_ADDRESS;

#define TOS_FRAME_TYPE bitread_frame
TOS_FRAME_BEGIN(radio_frame)
{
    char            primary[3];
    char            secondary[3];
    char            state;
    char            count;
    char            last_bit;
    char            waiting;
    char            wait_amount;
    unsigned int    shift_reg;
}
TOS_FRAME_END(radio_frame);

TOS_TASK(radio_encode_thread)
{
    // encode byte and store it into buffer.
    encodeData();
    // if this is the start of a transmisison, encode the start symbol.
    if (VAR(state) == 1) {

	// VAR(primary)[1] = 0x26; //start frame 0x00100110
	// VAR(primary)[2] = 0xb5; //start frame 0x10110101

	// VAR(primary)[0] = 0x1; //start frame 0x1
	// VAR(primary)[1] = 0x35; //start frame 0x00110101
	// VAR(primary)[2] = 0x15; //start frame 0xXXX10101
	// VAR(count) = 3;

	VAR(primary)[0] = 0x2;	// start frame 0x10
	VAR(primary)[1] = 0x6b;	// start frame 0x01101011
	VAR(primary)[2] = 0x05;	// start frame 0xXXXX0101
	VAR(count) = 4;


	VAR(state) = 4;		// there are now 2 bytes encoded.
    } else {
	VAR(state) = 4;		// there are now 2 bytes encoded.
    }

#ifdef FULLPC_DEBUG
    printf("radio_encode_thread running: %x, %x\n", VAR(secondary[1]),
	   VAR(secondary[2]));
#endif
}

TOS_TASK(radio_decode_thread)
{
#ifdef FULLPC_DEBUG
    printf("radio_decode_thread running: %x, %x\n", VAR(secondary[1]),
	   VAR(secondary[2]));
#endif
    // decode the byte that has been recieved.

    if (!(TOS_SIGNAL_EVENT(RADIO_BYTE_RX_BYTE_READY) (decodeData(), 0))) {
	// if the event returns false, then stop receiving, go to search
	// for the
	// start symbol at the high sampling rate.

	// VAR(state) = 0;
	// TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE)(0);
    }
}

char            TOS_COMMAND(RADIO_PACKET_OVER) () {
    VAR(state) = 0;
    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (0);
}

char            TOS_COMMAND(RADIO_BYTE_INIT) () {
    VAR(state) = 0;
    TOS_CALL_COMMAND(RADIO_SUB_INIT) ();
#ifdef FULLPC
    printf("Radio Byte handler initialized.\n");
#endif
    VAR(wait_amount) = 12;
    VAR(shift_reg) = 119 * TOS_LOCAL_ADDRESS;
    return 1;
}

char            TOS_COMMAND(RADIO_BYTE_TX_BYTES) (char data) {
#ifdef FULLPC_DEBUG
    printf("TX_bytes: state=%x, data=%x\n", VAR(state), data);
#endif
    if (VAR(state) == 0) {
	// if currently in idle mode, then switch over to transmit mode
	// and set state to waiting to transmit first byte.
	VAR(secondary[0]) = data;
	VAR(state) = 10;

#ifdef FULLPC
	VAR(waiting) = 9000;
	TOS_SIGNAL_EVENT(RADIO_BYTE_RX_BIT_EVENT) (0);
#endif

	return 1;
    } else if (VAR(state) == 2) {
	// if in the middle of a transmission and one byte is encoded
	// go to the one byte encoded and one byte in the encode buffer.
	VAR(state) = 3;
	VAR(secondary[0]) = data;
	// schedule the encode task.
	TOS_POST_TASK(radio_encode_thread);
	return 1;
    } else if (VAR(state) == 4) {
	return 0;
    }
    return 0;
}

// mode 1 = active;
// mode 0 = sleep;

char            TOS_COMMAND(RADIO_BYTE_PWR) (char mode) {
    if (mode == 0) {
	// if low power mode, tell lower components
	VAR(state) = 0;
	TOS_CALL_COMMAND(RADIO_SUB_PWR) (0);
    } else {
	// set the RMF component into "search for start symbol" mode.
	TOS_CALL_COMMAND(RADIO_SUB_PWR) (1);
	TOS_CALL_COMMAND(RADIO_SUB_RX_MODE) ();
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (0);
	VAR(state) = 0;
	VAR(count) = 0;
	VAR(last_bit) = 0xff;
	VAR(waiting) = 0;
    }
    return 1;
}


char            TOS_EVENT(RADIO_BYTE_TX_BIT_EVENT) () {
#ifdef FULLPC_DEBUG
    printf("radio tx bit event %d\n", VAR(primary)[2] & 0x1);
#endif
    // if we're not it a transmit state, return false.
    if (VAR(state) != 2 && VAR(state) != 3 && VAR(state) != 4)
	return 0;
    // send the next bit that we have stored.
    TOS_CALL_COMMAND(RADIO_SUB_TX_BIT) (VAR(primary)[2] & 0x01);
    // right shift the buffer.
    VAR(primary)[2] = VAR(primary)[2] >> 1;
    // increment our bytes sent count.
    VAR(count)++;
    if (VAR(count) == 8) {
	// once 8 have gone out, get ready to send out the nibble.
	VAR(primary)[2] = VAR(primary)[1];
    } else if (VAR(count) == 16) {
	VAR(primary)[2] = VAR(primary)[0];
    } else if (VAR(count) == 18) {
	if (VAR(state) == 4) {
	    // if another byte is ready, then shift the 
	    // data over to first and second.
	    VAR(primary)[0] = VAR(secondary)[0];
	    VAR(primary)[1] = VAR(secondary)[1];
	    VAR(primary)[2] = VAR(secondary)[2];

	    VAR(count) = 0;
	    VAR(state) = 2;	// now only one byte is bufferred.
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY) (1);	// fire
								// the
								// byte
								// transmitted 
								// event.
	} else {
	    // if there are no bytes bufferred, go back to idle.
	    VAR(state) = 0;
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_BYTE_READY) (1);
	    TOS_SIGNAL_EVENT(RADIO_BYTE_TX_DONE) ();
	    TOS_CALL_COMMAND(RADIO_SUB_RX_MODE) ();
	    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (0);
	}
    }
    return 1;
}

char            TOS_EVENT(RADIO_BYTE_RX_BIT_EVENT) (char data) {
    char            bit;
#ifdef FULLPC
    // because the FULLPC version doesn't do 2x sampling, we fake it out
    // with
    // this.
    VAR(last_bit) = data;
#endif
    if (VAR(state) == 0) {
	// if we are in the idle state, the check if the bit read
	// matches the last bit.
	if (VAR(last_bit) != data) {
	    VAR(last_bit) = data;
	    return 1;
	}
	// if so, set last bit to invalid,
	VAR(last_bit) = 0xff;
	// right shift previously read data.
	VAR(primary)[2] >>= 1;
	// mask out upper bit
	VAR(primary)[2] &= 0x7f;
	// if lowest bit of first is one, store it in second
	if (VAR(primary)[1] & 0x1)
	    VAR(primary)[2] = VAR(primary)[2] | 0x80;
	// don't forget that the start symbol is only 9 bits long. 
	VAR(primary)[1] = data & 0x1;
#ifdef FULLPC_DEBUG
	printf("checking for start symbol: %x, %x\n",
	       VAR(primary)[1] & 0xff, VAR(primary)[2] & 0xff);
#endif
	// if you now have the start symbol, go to the waiting for first
	// bit state.
	if (VAR(primary)[1] == (char) 0x1
	    && VAR(primary)[2] == (char) 0x35) {
	    VAR(state) = 5;
	    VAR(count) = 0;
	    // set bit rate so next sample falls in middle of next
	    // transmitted bit.
	    TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (1);
	}

    } else if (VAR(state) == 5) {
	// just read first bit.
	// set bit rate to match TX rate.
	TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (2);
	VAR(state) = 6;
	VAR(count) = 1;
	// store it.
	if (data) {
	    VAR(primary)[1] = 0x80;

	} else {
	    VAR(primary)[1] = 0;
	}
    } else if (VAR(state) == 6) {
	// clock in bit.
	VAR(count)++;
	VAR(primary)[1] >>= 1;
	VAR(primary)[1] &= 0x7f;
	if (data) {
	    VAR(primary)[1] |= 0x80;
	}
	if (VAR(count) == 8) {
	    VAR(secondary[2]) = VAR(primary[1]);
	} else if (VAR(count) == 16) {
	    VAR(count)++;
	    // sore the encoded data into a buffer.
	    VAR(secondary)[1] = VAR(primary)[1];
	    VAR(state) = 7;
	    // scheduled the decode task.
	    // TOS_POST_TASK(radio_decode_thread);
	}
    } else if (VAR(state) == 7) {
	VAR(secondary)[0] = data;
	VAR(state) = 8;
    } else if (VAR(state) == 8) {
	// throw away the higest bit.
	VAR(state) = 5;
	// scheduled the decode task.
	TOS_POST_TASK(radio_decode_thread);
#ifdef FULLPC_DEBUG
	printf("entire byte received: %x, %x\n", VAR(secondary)[1],
	       VAR(secondary)[2]);
#endif
    } else if (VAR(state) == 10) {
	// waiting for channle to be idle.
	if (data) {
	    // if we just read activity, then reset the waiting counter.
	    VAR(waiting) = 0;
	} else {
	    if (++VAR(waiting) == 11) {
		bit = (VAR(shift_reg) & 0x2) >> 1;
		bit ^= ((VAR(shift_reg) & 0x4000) >> 14);
		bit ^= ((VAR(shift_reg) & 0x8000) >> 15);
		VAR(shift_reg) >>= 1;
		if (bit & 0x1)
		    VAR(shift_reg) |= 0x8000;
		VAR(wait_amount) = (VAR(shift_reg) & 0x3f) + 12;
	    }
	    // if we've not heard anything for 8 samples then...
	    if (VAR(waiting) > VAR(wait_amount)) {
		// go to the transmitting state.
		VAR(state) = 1;
		VAR(waiting) = 0;
		// schedule task to start transfer, set TX_mode, and set
		// bit rate
		TOS_CALL_COMMAND(RADIO_SUB_TX_MODE) ();
		TOS_CALL_COMMAND(RADIO_SUB_SET_BIT_RATE) (2);

		TOS_POST_TASK(radio_encode_thread);
	    }
	}
    }
    return 1;
}



void
encodeData()
{
    char            ret_high = 0;
    char            ret_low = 0;
    char            ret_mid = 0;
    char            val = VAR(secondary[0]);
    if ((val & 0x1) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x0;
	ret_low ^= 0x77;
    }
    if ((val & 0x2) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x1;
	ret_low ^= 0x34;
    }
    if ((val & 0x4) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x2;
	ret_low ^= 0x32;
    }
    if ((val & 0x8) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x8;
	ret_low ^= 0x31;
    }
    if ((val & 0x10) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x10;
	ret_low ^= 0x26;
    }
    if ((val & 0x20) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x60;
	ret_low ^= 0x25;
    }
    if ((val & 0x40) != 0) {
	ret_high ^= 0;
	ret_mid ^= 0x80;
	ret_low ^= 0x13;
    }
    if ((val & 0x80) != 0) {
	ret_high ^= 0x1;
	ret_mid ^= 0;
	ret_low ^= 0x7;
    }

    if ((ret_low & 0xc) == 0)
	ret_low |= 0x8;
    if ((ret_low & 0x40) == 0 && (ret_mid & 0x1) == 0)
	ret_low |= 0x80;
    if ((ret_mid & 0xa) == 0)
	ret_mid |= 0x4;
    if ((ret_mid & 0x50) == 0)
	ret_mid |= 0x20;
    if ((ret_high & 0x1) == 0)
	ret_high |= 0x2;


    VAR(secondary[0]) = ret_high;
    VAR(secondary[1]) = ret_mid;
    VAR(secondary[2]) = ret_low;

}

char
decodeData()
{

    // strip the data
    char            ret_high = 0;
    char            ret_low = 0;
    char            val,
                    val2,
                    output;


    ret_high = (char) ((VAR(secondary)[0] << 4) & 0x10);
    ret_high |= (char) ((VAR(secondary)[1] >> 4) & 0xc);
    ret_high |= (char) ((VAR(secondary)[1] >> 3) & 0x3);
    ret_low = (char) ((VAR(secondary)[1] << 6) & 0xc0);
    ret_low |= (char) ((VAR(secondary)[2] >> 1) & 0x38);
    ret_low |= (char) (VAR(secondary)[2] & 0x7);
    // check the data
    val = ret_low;
    val2 = ret_high;
    output = 0;
    if ((val & 0x1) != 0)
	output ^= 0x1;
    if ((val & 0x2) != 0)
	output ^= 0x2;
    if ((val & 0x4) != 0)
	output ^= 0x4;
    if ((val & 0x8) != 0)
	output ^= 0x8;
    if ((val & 0x10) != 0)
	output ^= 0x10;
    if ((val & 0x20) != 0)
	output ^= 0x1f;
    if ((val & 0x40) != 0)
	output ^= 0x1c;
    if ((val & 0x80) != 0)
	output ^= 0x1a;
    if ((val2 & 0x1) != 0)
	output ^= 0x19;
    if ((val2 & 0x2) != 0)
	output ^= 0x16;
    if ((val2 & 0x4) != 0)
	output ^= 0x15;
    if ((val2 & 0x8) != 0)
	output ^= 0xb;
    if ((val2 & 0x10) != 0)
	output ^= 0x7;
    if (output == 0) {
    } else if (output == 0x1) {
	val ^= 0x1;
    } else if (output == 0x2) {
	val ^= 0x2;
    } else if (output == 0x4) {
	val ^= 0x4;
    } else if (output == 0x8) {
	val ^= 0x8;
    } else if (output == 0x10) {
	val ^= 0x10;
    } else if (output == 0x1f) {
	val ^= 0x20;
    } else if (output == 0x1c) {
	val ^= 0x40;
    } else if (output == 0x1a) {
	val ^= 0x80;
    } else if (output == 0x19) {
	val2 ^= 0x1;
    } else if (output == 0x16) {
	val2 ^= 0x2;
    } else if (output == 0x15) {
	val2 ^= 0x4;
    } else if (output == 0xb) {
	val2 ^= 0x8;
    } else if (output == 0x7) {
	val2 ^= 0x10;
    }
    // pull off the data bits
    output = (char) ((val >> 5) & 0x7);
    output |= ((char) val2 << 3) & 0xf8;
    return output;
}
