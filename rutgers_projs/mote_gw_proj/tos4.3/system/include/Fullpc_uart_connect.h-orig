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

#include <stdio.h>
//#include <tcpd.h>
#include <strings.h>
#include <malloc.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
//#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "sys/socket.h"
#include "sys/types.h"
#include "netinet/in.h"

int uart_send;
int uart_initialized = 0;
void udp_init_socket(){
    char* host = "127.0.0.1";
    int port = 8765;
    int connection;
    struct hostent *he;
    struct sockaddr_in serv_addr;
    unsigned long addr;
    int err;
    if(uart_initialized == 1) return;
    uart_initialized = 1;
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons((unsigned short)port);
	printf("looking up host\n");
	he = gethostbyname(host);
	printf("done looking up base\n");
	addr =(unsigned long)
	    (((unsigned long)he->h_addr_list[0][0] & 0xFF)<<24L)|
	   (((unsigned long)he->h_addr_list[0][1] & 0xFF)<<16L)|
	    (((unsigned long)he->h_addr_list[0][2] & 0xFF)<<8L)|
	    (((unsigned long)he->h_addr_list[0][3] & 0xFF));
	serv_addr.sin_addr.s_addr=htonl(addr);
	connection = socket(AF_INET, SOCK_STREAM, 0);
	printf("connection #%d\n", connection);
	if(connection >= 0){
		printf("socket created.\n");
	}else{
		printf("socket create FAILED: %d, %d\n", errno, connection);
	}
	err = connect(connection, &serv_addr, sizeof(struct sockaddr));
	if(0 == err){
	  printf("connected\n");
	}else{
	  printf("connection failed  : %d : %d\n", err, errno);
	  printf("could not connect to uart sink manager.\n");
	  connection = 0;
	}
	uart_send = connection;
  }
