/*
 * Copyright (c) 1999 The Regents of the University 
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
 * Author:	U.C. Berkeley Millennium Project
 * File:	timing.h
 * Revision:	$Revision: 1.2 $
 *
 */

#ifndef VIA_TIMING_H
#define VIA_TIMING_H

/* Some timing macros */

//#define PENTIUM_MHZ 400.0

#ifdef PENTIUM_MHZ

#ifdef _WIN32
double get_seconds() {
  unsigned long f1, f2;
  __int64 now;
  __asm {
    __asm _emit 0x0f
    __asm _emit 0x31
    __asm mov f1, eax
    __asm mov f2, edx
  }
  now = f1 | ((__int64)f2 << 32);
  return (double)now / (double)(1e6*PENTIUM_MHZ);
}
#endif

#ifdef linux

static inline double get_seconds(void) {
  unsigned long long now;
  __asm __volatile (".byte 0x0f, 0x31" : "=A" (now));
  return (double)((double)now / (1.0e6*PENTIUM_MHZ));
}
#endif

#else /* PENTIUM_MHZ not defined */

#ifdef _WIN32

#if 0
static __inline double get_seconds(void) {
  DWORD t;
  t = GetTickCount();
  return (double)t/(double)1.0e3;
}

#else

#include <winbase.h>
static __inline double get_seconds(void) {
  LONGLONG ullFrequency,ullCount;
  QueryPerformanceFrequency((LARGE_INTEGER *)&ullFrequency);
  QueryPerformanceCounter((LARGE_INTEGER *)&ullCount);

  return ((double)ullCount/(double)ullFrequency);
}
#endif

#endif

#ifdef linux
#include <sys/time.h>

static inline double get_seconds(void) {
  struct timeval t;
  gettimeofday(&t,NULL);
  return (double)t.tv_sec+((double)t.tv_usec/(double)1e6);
}
#endif

#endif

void set_timings_file (char * file);

void mark_time (char * string);

void delta_time(int delta, char* string);

void print_timings (void);


#endif

/* 
 * $Log: timing.h,v $
 * Revision 1.2  2002/04/08 16:51:47  ashwink
 *
 * Dont panic!
 * Did a general cleanup to remove characters after #endif
 *
 * Revision 1.1.1.1  2002/03/08 23:52:38  ashwink
 *
 * APS Code includes:
 * 	mote code
 * 	mote-gw
 * 	tcl/tk demo files
 *
 * Revision 1.1.1.1  2001/11/07 23:28:24  ashwink
 * Mote Gateway
 *
 * Revision 1.1  2000/10/19 01:09:05  jhill
 * *** empty log message ***
 *
 * Revision 1.1  2000/04/26 01:04:57  philipb
 * *** empty log message ***
 *
 * Revision 1.1  2000/04/25 21:13:51  philipb
 * *** empty log message ***
 *
 * Revision 1.4  1999/09/22 19:30:18  philipb
 * Revised pingpong.  Removed debugging code.
 *
 * Revision 1.3  1999/09/06 23:59:45  philipb
 * Bug fixes
 *
 * Revision 1.2  1999/08/20 02:28:11  philipb
 * Added code for NT to utilize high resolution timers.
 *
 * Revision 1.1  1999/07/08 02:17:22  philipb
 * *** empty log message ***
 *
 * Revision 1.0.0.1  1999/06/23 22:31:13  philipb
 * Initial Import
 *
 */
