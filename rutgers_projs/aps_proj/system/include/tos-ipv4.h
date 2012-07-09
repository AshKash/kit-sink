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
 * Version:                     $Id: tos-ipv4.h,v 1.2 2002/04/08 16:51:51 ashwink Exp $
 * Creation Date:               Tue Aug 14 17:02:20 2001
 * Filename:                    tos-ipv4.h
 * History:
 * $Log: tos-ipv4.h,v $
 * Revision 1.2  2002/04/08 16:51:51  ashwink
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
 * Revision 1.3  2002/01/29 02:28:05  xili
 * working udp echo code
 *
 * Revision 1.2  2002/01/09 23:26:29  xili
 * add the definition for IP BROADCAST address
 *
 * Revision 1.1.1.1  2001/10/04 04:12:10  rmartin
 * hopefully this is correct
 *
 * Revision 1.2  2001/09/19 04:31:21  rmartin
 * Added byteswap routines
 *
 * Fixed some init routines. More init routines will be needed
 * Also, this version compiles for the 1st time.
 *
 * more fixeds with byteswap
 *
 * Revision 1.1  2001/09/19 01:53:14  rmartin
 *
 * Placing this file in the correct place, makefile nukes all .h files
 *
 * Revision 1.1  2001/09/17 03:10:54  rmartin
 * added this header file
 *
 */

/* note: this assumes the machine is little-endian */ 
/* this should apply for the forseeable furture of TOS */

#ifndef TOS_IPV4_H 
#define TOS_IPV4_H 1

#include <inttypes.h>

#ifndef FULLPC
/* define these to take the correct types */
extern uint32_t ntohl( uint32_t val);
extern uint16_t ntohs( uint16_t val);
extern uint32_t htonl( uint32_t val);
extern uint16_t htons( uint16_t val);

/* a 16 bit byte-swap */
#define bswap16(x)  ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define bswap32(x)  ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |\
                   (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))


/* little endian-ness means we must byteswap */
#define ntohl(x)   bswap32(x)
#define ntohs(x)   bswap16(x)
#define htonl(x)   bswap32(x)
#define htons(x)   bswap16(x)

#endif

#define MAX_TTL  64   /* from RFC 1340 */
#define	IP_MSS	 576  /* maximum segment size */
#define UDP_PROTOCOL_ID 17 

#define IPV4_BCAST_ADDR 0xffffffff
#define IPV4_BOOT_ADDR 0x0
//#define IPV4_BCAST_ADDR1 0xffffffff
//#define IPV4_BCAST_ADDR2 0x0

typedef uint32_t ipv4addr_t; 

/* the TOS AM header */ 
typedef struct toshdr_s {
  char addr; 
  char type; 
  char group; 
} toshdr_t; 

/* IP header */
typedef struct ipv4hdr_s {
  uint8_t hl_version;  /* header length & version*/ 
  uint8_t tos;                    /* type of service */
  uint16_t len;                     /* total length */
  uint16_t id;                      /* identification */
  uint16_t off;                     /* fragment offset field */
  uint8_t ttl;                    /* time to live */
  uint8_t protocol;               /* protocol */
  uint16_t check;                   /* checksum */
  uint32_t src_addr;
  uint32_t dest_addr;      /* source and dest address */
} ipv4hdr_t; 


/* the UDP header */
typedef struct udphdr_s {
  uint16_t     src_port;       /* source port */
  uint16_t     dst_port;       /* destination port */
  uint16_t     len;            /* length of UDP packet */
  uint16_t     check;          /* 16 bit checksum */
} udphdr_t;

/* A TCP header */ 
typedef struct tcphdr_s {
  uint16_t source;
  uint16_t dest;
  uint32_t seq;
  uint32_t ack_seq;
  uint16_t window;
  uint16_t check;
  uint16_t urg_ptr;
} tcphdr_t;



#endif


