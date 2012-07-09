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

#ifndef __TOS_H__
#define __TOS_H__


/* 

A TOS component implements a collection of commands and handlers.  It  may 
contain a frame and/or threads.  It may use commands in other, usually 
subordinate components, and it may signal handlers in other, usually superior,
components.  Storage within the component frame is referenced using the VAR 
macro.

The current implementation of the TOS structuring macros is quite
primitive, relying entirely on C function handling.  Future versions are
expected to utilize more sophisticated techniques.

*/

/* Declare a frame */

#define TOS_FRAME_BEGIN(frame_type) typedef struct
#define TOS_FRAME_END(frame_type) frame_type; \
static frame_type TOS_MY_Frame

/* Declare a thread: task, command, or handler */
#define TOS_TASK(task_name) void task_name##_TASK()
#define TOS_COMMAND(command_name) command_name##_COMMAND
#define TOS_EVENT(event_name) event_name##_EVENT
#define TOS_MSG_EVENT(event_name) event_name##_EVENT

/* Post a thread: */
#define TOS_POST_TASK(task_name) TOS_post(task_name##_TASK, &TOS_MY_Frame) 
#define TOS_SIGNAL_EVENT(event_name) event_name##_EVENT
#define TOS_CALL_COMMAND(command_name) command_name##_COMMAND

/* Access a frame variable */
#define VAR(x) TOS_MY_Frame.x

#define AM_MSG(x) x##_EVENT__AM_DISPATCH
#define MSG_DATA(x) x.data



#include "hardware.h" //take care of some hardware specific stuff.
#include "sched.h"
#include "am_dispatch.h"

#ifdef FULLPC
#include <stdio.h>
#endif
#endif
