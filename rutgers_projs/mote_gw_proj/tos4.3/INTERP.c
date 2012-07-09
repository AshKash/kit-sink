/*									tab:4
 * INTERP.c - simple byte-code interpreter
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
 * Authors:   David Culler
 * History:   created 10/30/2000
 *
 *
 */

#include "tos.h"
#include "INTERP.h"
#include "instruc.h"

#define MEMSIZE 32

/* stack ops - should check bounds */
#define PUSH(x) (push(x))
#define POP() (pop())

/* State encoding */
#define idle 0
#define running 1
#define waitdata 2
#define waitsend 3
#define restart 4

#define TOS_FRAME_TYPE INTERP_frame
TOS_FRAME_BEGIN(INTERP_frame) {
  int state;			/* Component counter state */
  int light;			/* Recent light reading */
  int clocks;
  char ip;
  char codesize;
  char sp;
  char mem[MEMSIZE];
}
TOS_FRAME_END(INTERP_frame);

/* 

Stack starts from end of mem and grows upward as INTS.
   sp points to last full.

 */

void push(int x) {
  VAR(sp) = VAR(sp) - sizeof(int);
  *(int *) &VAR(mem)[(int) VAR(sp)] = x;
}

int pop() {
  int p = *(int *) &VAR(mem)[(int) VAR(sp)];
  VAR(sp) = VAR(sp) + sizeof(int);
  return p;
}

extern const char LOCAL_ADDR_BYTE_1; 
extern const char TOS_LOCAL_ADDRESS;

/* INTERP_INIT:  
   flash the LEDs
   initialize lower components.
   initialize component state, including constant portion of msgs.
*/
char TOS_COMMAND(INTERP_INIT)(){
  TOS_CALL_COMMAND(INTERP_SUB_INIT)();       /* initialize lower components */
  VAR(state)    = 0;
  VAR(light)    = 0;
  VAR(codesize) = 0;
  VAR(sp)       = 0;
  VAR(ip)       = 0;
  printf("INTERP initialized\n");
  return 1;
}

void LEDop(cmd) {
  char op = (cmd >> 3) & 3;
  char led = cmd & 7;
  switch (op) {
  case 0:			/* set */
    if (led & 1) TOS_CALL_COMMAND(INTERP_LEDr_on)();
    else TOS_CALL_COMMAND(INTERP_LEDr_off)();
    if (led & 2) TOS_CALL_COMMAND(INTERP_LEDg_on)();
    else TOS_CALL_COMMAND(INTERP_LEDg_off)();
    if (led & 4) TOS_CALL_COMMAND(INTERP_LEDy_on)();
    else TOS_CALL_COMMAND(INTERP_LEDy_off)();
  case 1:			/* OFF 0 bits */
    if (!(led & 1)) TOS_CALL_COMMAND(INTERP_LEDr_off)();
    if (!(led & 2)) TOS_CALL_COMMAND(INTERP_LEDg_off)();
    if (!(led & 4)) TOS_CALL_COMMAND(INTERP_LEDy_off)();
  case 2:			/* on 1 bits */
    if (led & 1) TOS_CALL_COMMAND(INTERP_LEDr_on)();
    if (led & 2) TOS_CALL_COMMAND(INTERP_LEDg_on)();
    if (led & 4) TOS_CALL_COMMAND(INTERP_LEDy_on)();
  case 3:			/* TOGGLE 1 bits */
    if (led & 1) TOS_CALL_COMMAND(INTERP_LEDr_toggle)();
    if (led & 2) TOS_CALL_COMMAND(INTERP_LEDg_toggle)();
    if (led & 4) TOS_CALL_COMMAND(INTERP_LEDy_toggle)();
  }

}



/* INTERP_START command:
*/
char TOS_COMMAND(INTERP_START)(){
  return 1;
}

/* run - run interpeter from current state
 */

TOS_TASK(run) {
  char op;
  int arg1,arg2,arg3=0;

  while (VAR(ip) < VAR(codesize)) {
    op = VAR(mem[(int)VAR(ip)]);
    printf("Exec ip:%2d, op:%2x, sp:%2d, tos: %d state:%d\n", VAR(ip), op&0xff, 
	   VAR(sp), *((int *)&VAR(mem)[(int)VAR(sp)]), VAR(state));

#ifndef FULLPC
    /*
    TOS_CALL_COMMAND(INTERP_SEND_HOST)(0xFF, VAR(ip), op, VAR(sp), 
				       *(int *)&VAR(mem[VAR(sp)]), 
				       *(int *)&VAR(mem[VAR(sp)+1]), 
				       VAR(clocks),VAR(state));
    */
#endif
    if (op == OPhalt) {
      VAR(state) = idle; return;	
    } 
    else if (xclass(op)) {
      arg1 = xarg(op);
      printf("xclass: op %d\n", arg1);
      switch (op & xopmask) {
      case OPpushc: PUSH(arg1); break;
      case OPpush : PUSH(*(int *)&VAR(mem)[arg1]); break;
      case OPcond : arg2 = POP(); if (arg2) VAR(ip) = VAR(ip)+arg1-1;
      }
    }
    else if (binaryop(op)) {
      arg1 = POP();		/* TOS */
      arg2 = POP();		/* POS */
      switch (op) {
      case OPadd : arg3 = arg2 + arg1; break;
      case OPsub : arg3 = arg2 - arg1; break;      
      case OPsubr: arg3 = arg1 - arg2; break;           
      case OPand : arg3 = arg2 & arg1; break;           
      case OPor  : arg3 = arg2 | arg1; break;           
      case OPxor : arg3 = arg2 ^ arg1; break;           
      case OPshift : arg3 = (arg1 > 0) ? (arg2 << arg1) : (arg2 >> -arg1);
      }
      printf("Binary: %d := %d op %d\n", arg3,arg2,arg1);
      PUSH(arg3);
    }    
    else {
      arg1 = POP();
      printf("Unary: op %d\n", arg1);
      switch (op) {
      case OPputled   : LEDop(arg1); break; 
      case OPjump     : VAR(ip) = arg1-1; break;
      case OPcall     : PUSH(VAR(ip)); VAR(ip) = arg1-1; break;
      case OPreturn   : VAR(ip) = arg1; break;
      case OPpop      : break;
      case OPcopy     : PUSH(arg1); PUSH(arg1); break;
      case OPgetsensor:		/* arg1 currently ignored */
	if (VAR(state) == waitdata) { /* resume from get_data */
	  PUSH(VAR(light));	
	  VAR(state) = running;
	  break;
	} else if (TOS_CALL_COMMAND(INTERP_GET_DATA)())
	  {		/* start data reading */
	    VAR(state) = waitdata;
	    return;		/* suspend current instruction */
	  }
	else {
	  VAR(state) = idle;	/* abort - sensor busy*/
	  return;
	}
      case OPsenddata:		/* send top. marg unused */
	if (!TOS_CALL_COMMAND(INTERP_SEND_DATA)(arg1)) {
	  if (VAR(state) == running) {
	    VAR(state) = waitsend;	/* retry once - send busy*/
	    return;
	  }
	  VAR(state) = running; /* give up on this send */
	}
	break;
      case OPstore : 
	arg2 = POP(); 
	*(int *)&VAR(mem)[arg1] = arg2;  break;
      }
    }
    VAR(ip)++;
    TOS_POST_TASK(run);		/* allow other task before next inst. */
    return;
  }
  VAR(state) = idle;
  return;
}


/*  INTERP_DATA_EVENT(data):
    handler for subsystem data event, fired when data ready.

    store data in frame, resume interpreter
    it should be in waitdata state
 */

char TOS_EVENT(INTERP_DATA_EVENT)(int data){
  printf("INTERP data event\n");
  VAR(light) = data;
  TOS_POST_TASK(run);  
  return 1;
}

/* Clock Event Handler  */

void TOS_EVENT(INTERP_CLOCK_EVENT)(){
  printf("INTERP clock event\n");
  VAR(clocks)++;
  if (!VAR(codesize)) {
    TOS_CALL_COMMAND(INTERP_LEDr_toggle)();
  } else if (VAR(state) == idle) {
    VAR(state) = running;
    VAR(ip) = 0;
    VAR(sp) = MEMSIZE;
    TOS_POST_TASK(run);
  } else if (VAR(state) == waitsend) {
    TOS_POST_TASK(run);
  }
}

/* INTERP_CAPSULE_EVENT
 * 0 - len
 * 1:len-1 bytes of code
 */

char TOS_EVENT(INTERP_CAPSULE_EVENT)(char* data){
  int i, len;
  char sum=0;
  //  TOS_CALL_COMMAND(INTERP_LEDg_on)();		/* green */
  len = data[0];
  for (i=0; i<len; i++) {
    VAR(mem)[i]=data[i+1];
    sum += data[i+1];
  }
  VAR(codesize) = len;
  VAR(state) = idle;
  /* try to echo the pgm msg checksum back to host */
  //TOS_CALL_COMMAND(INTERP_SEND_DATA)(sum);
  return 1;
}













