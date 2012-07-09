/*									tab:4
 *     Byte-code instruction definitions for mote byte-class interpreter
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
 *  Author: David E. Culler
 *  Date:   Sept, 30, 2000
 *
 */

/*   Op code formats:

   base opcode |00000000|  0x00
          thru |00111111|  0x3f
   xclass      |01xxxxxx|  0x40
          thru |11xxxxxx|  0xFF

instructions semantics

 0  OPhalt      0x00      00000000

 1  OPadd       0x01      00000001   POS <= POS + TOS
 2  OPsub       0x02      00000010   POS <= POS - TOS
 3  OPsubr      0x03      00000011   POS <= TOS - POS 
 4  OPand       0x04      00000100   POS <= POS & TOS 
 5  OPor        0x05      00000101   POS <= POS | TOS
 6  OPxor       0x06      00000110   POS <= POS ^ TOS  
 7  OPshift     0x07      00000111   POS <= POS << TOS (signed)
 
 8  OPputled    0x08      00001000   TOS used as 2-bit cmd + 3-bit oprnd
 9  OPjump      0x09      00001001   IP <= TOS
10  OPcall      0x0a      00001010   IP <= TOS & TOS <= IP
11  OPreturn    0x0b      00001011   IP <= TOS+1
12  OPpop       0x0c      00001100   pop
13  OPgetsensor 0x0d      00001101   TOS <= sensor(TOS)
14  OPsenddata  0x0e      00001110   send(TOS)
15  OPstore     0x0f      00001111   MEM(TOS) <= POS
16  OPcopy      0x10      00010000   NOS <= TOS
XCLASS
64  OPpushc     0x40-7f   01xxxxxx   NOS <= signex(xarg)
128 OPpush      0x80-BF   10xxxxxx   TOS <= MEM(TOS)
192 OPcond      0xc0-ff   11xxxxxx   if (POS) ip <= TOS

Memory Model

0 - n-1:  byte-code instructions
            if idle, clock event handler schedules IP=0, SP=EOM
n - ?     static data (int)
? - MAX   stack (int)
            grows upward.  TOS points to last full

*/

#define xclass(op) (op & 0xC0)

/*
   Base clase |0000oooo|
*/

#define OPhalt      0x00

/* Basic Binary Operators with result*/
#define OPadd       0x01
#define OPsub       0x02
#define OPsubr      0x03
#define OPand       0x04
#define OPor        0x05
#define OPxor       0x06
#define OPshift     0x07
#define binaryop(op) (op && (op <= OPshift))


/* Unary Operators */
#define OPputled    0x08
#define OPjump      0x09
#define OPcall      0x0a
#define OPreturn    0x0b
#define OPpop       0x0c
#define OPgetsensor 0x0d

/* Binary no result */
#define OPsenddata  0x0e

#define OPstore     0x0f
#define OPcopy      0x10

/* 0x11 - 0x3f unused */

/* Opclass X are of the form |oo|xxxxxx|
   where oo is not 00
*/

#define OPpushc 0x40
#define OPpush  0x80
#define OPcond  0xc0

#define xmask    0x3F
#define xsignbit 0x20
#define xopmask  0xC0
#define xarg(op)   ((op & xsignbit) ? (op | (-1 ^ xmask)) : (op & xmask))




