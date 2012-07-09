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

#define SUB_ADD_UART
#define NEW_SCHED
// #define TEST_MOD

/*
 * Scheduling data structure
 * 
 * cyclic buffer of size MAX_THREADS TOS_sched_full: index of first full
 * slot TOS_sched_free: index of first free slot empty if free == full
 * overflow if advancing free would make full == free
 * 
 * Each entry consists of a status field, frame pointer, and thread
 * pointer 
 */

#define TOS_Q_FREE 0;
#define TOS_Q_READY 1;
#define TOS_Q_ACTIVE 2;

typedef struct {
    // int status;
    void            (*tp) ();
    // void *fp;
} TOS_sched_entry_T;


#define MAX_THREADS 6
TOS_sched_entry_T TOS_queue[MAX_THREADS];
// char TOS_sched_full, TOS_sched_free = 0;
// volatile char TOS_sched_full, TOS_sched_free = 0;
volatile char   TOS_sched_full = 0,
                TOS_sched_free = 0;

#ifdef TEST_MOD
volatile char   TOS_last_free = MAX_THREADS - 1;
#endif

#define EMPTY (TOS_sched_full == TOS_sched_free)
#define ADVANCE(ptr) ptr = (ptr+1 == MAX_THREADS) ? 0 : ptr+1

#ifdef TEST_MOD
char
get_prev(char num)
{
    return ((num + MAX_THREADS - 1) % MAX_THREADS);
}
#endif

/*
 * TOS_empty
 * 
 * return NULL is queue is non-empty
 * 
 */

int
TOS_empty()
{
    return (EMPTY);
}

/*
 * TOS_post (thread_pointer, frame_pointer)
 * 
 * Post the associated thread to the next free slot Error of scheduling
 * queue overflows
 * 
 */

static inline char
compare_and_swap_double(char *x, char y, char z, void (**a) (),
			void (*b) (), void (*c) ())
{
#ifndef FULLPC
    char            prev = inp(SREG) & 0x80;
    cli();
#else
#endif
    if (*x == y && *a == b) {
	*x = z;
	*a = c;
#ifndef FULLPC
	if (prev)
	    sei();
#endif
	return 0x1;
    }
#ifndef FULLPC
    if (prev)
	sei();
#endif
#ifdef SUB_ADD_UART
    /*
     * this is not right, but hope would give me some idea for debugging 
     */
    sbi(USR, TXC);
    outp(TOS_sched_full, UDR);
    // outp(TOS_sched_free, UDR); 
    /*
     * if(*x != y && *a != b) outp(6, UDR); else if(*a != b) outp(8,
     * UDR); else if(*x != y) outp(7, UDR); 
     */
#endif
    return 0;
}


#ifndef NEW_SCHED

void
TOS_post(void   (*tp) (), void *fp)
{
#ifdef DEBUG
    printf("TOS_post: %d %x %x \n", TOS_sched_free, tp, fp);
#endif

    char            tmp = TOS_sched_free;
    // TOS_queue[TOS_sched_free].status = TOS_Q_READY;
    // TOS_queue[TOS_sched_free].tp = tp;
    // TOS_queue[TOS_sched_free].fp = fp;
#ifdef SUB_ADD_UART
    while (!compare_and_swap_double
	   (&TOS_sched_free, tmp, (tmp + 1 == MAX_THREADS) ? 0 : tmp + 1,
	    &(TOS_queue[(int) tmp].tp), 0, tp)) {
	sbi(USR, TXC);
	outp(TOS_sched_free, UDR);
	tmp = TOS_sched_free;
    }
#else
    while (!compare_and_swap_double
	   (&TOS_sched_free, tmp, (tmp + 1 == MAX_THREADS) ? 0 : tmp + 1,
	    &(TOS_queue[(int) tmp].tp), 0, tp))
	tmp = TOS_sched_free;
#endif
    // if (EMPTY) {
    // fprintf(stderr,"Attempt to overflow task queue\n");
    // exit(1);
    // }

}
#else


char
TOS_post(void   (*tp) (), void *fp)
{
#ifdef DEBUG
    printf("TOS_post: %d %x %x \n", TOS_sched_free, tp, fp);
#endif

    char            return_val = 2;
    unsigned char   tmp;
    char            prev;
    char            org;
    while (return_val == 2) {
	prev = inp(SREG) & 0x80;
	tmp = TOS_sched_free;
	org = tmp;
	tmp++;
	if (tmp == MAX_THREADS)
	    tmp = 0;
	if (tmp == TOS_sched_full) {
#ifdef SUB_ADD_UART
	    // sbi(USR, TXC);
	    // outp(TOS_sched_free, UDR); 
#endif
	    return_val = 0;
	} else {
	    cli();
	    if (org == TOS_sched_free) {
#ifdef TEST_MOD
		if (TOS_last_free != get_prev(TOS_sched_free)) {
		    sbi(USR, TXC);
		    outp(11, UDR);
		}
		TOS_last_free = TOS_sched_free;
#endif

#ifdef SUB_ADD_UART
		// sbi(USR, TXC);
		// outp(TOS_sched_free, UDR); 
#endif
		TOS_sched_free = tmp;
		TOS_queue[org].tp = tp;
		return_val = 1;
	    }
	    if (prev)
		sei();
	}
    }
    return return_val;
    /*
     * char tmp; while(!compare_and_swap_double(&TOS_sched_free,tmp,
     * (tmp+1 == MAX_THREADS) ? 0 : tmp+1, &(TOS_queue[(int)tmp].tp), 0,
     * tp)) tmp = TOS_sched_free; 
     */

}

#endif


/*
 * TOS_schedule_task()
 * 
 * If the queue is non-empty, remove the next task and execute it, freeing 
 * the queue entry.
 * 
 * return NULL is some task was ready and executed. return -1 if empty.
 * 
 */

/*
 * Schedule next task, return NULL if some task ready 
 */
int
TOS_schedule_task()
{
    int             old_full = TOS_sched_full;
    if (EMPTY) {
#ifdef DEBUG
	printf("TOS_schedule_task: %d empty \n", TOS_sched_full);
#endif
	return -1;
    }
#ifdef DEBUG
    printf("TOS_schedule_task: %d %d %x %x \n",
	   TOS_sched_full,
	   TOS_queue[old_full].status,
	   TOS_queue[old_full].tp, TOS_queue[old_full].fp);
#endif
    ADVANCE(TOS_sched_full);
#ifdef OLDSTUFF
    TOS_queue[old_full].status = TOS_Q_ACTIVE;
    TOS_queue[old_full].tp(TOS_queue[old_full].fp);
    TOS_queue[old_full].status = TOS_Q_FREE;
#endif
#ifdef SUB_ADD_UART
    // sbi(USR, TXC);
    // outp(old_full, UDR); 
#endif
    TOS_queue[old_full].tp();
    TOS_queue[old_full].tp = 0;
    return 0;
}
