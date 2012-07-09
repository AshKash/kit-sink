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
 * Authors:		Ashwin Kashyap
 * 
 * Changes:
 * This will now directly open the /dev/ttyS0 on the PC rather than try to
 * connect to a server
 *
 */

#include <stdio.h>
//#include <tcpd.h>
#include <strings.h>
#include <malloc.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <termios.h>
#include "../../tools/timing.h"
#include <sys/time.h>

#define BAUDRATE B19200

int uart_send;
int uart_initialized = 0;
void udp_init_socket(){
    int err;
    int com1;
    struct termios oldtio, newtio;

    if(uart_initialized == 1) return;
    uart_initialized = 1;

    /* open com1 for read/write */ 
    com1 = open("/dev/ttyS0", O_RDWR|O_NOCTTY|O_SYNC);
    if (com1 == -1) perror(": com1 open fails\n");
    printf("com1 open ok\n");
    
    /* save old serial port setting */
    tcgetattr(com1, &oldtio);
    bzero(&newtio, sizeof(newtio));
    
    //  newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    
    /* Raw output */
    newtio.c_oflag = 0;
    
    tcflush(com1, TCIFLUSH);
    tcsetattr(com1, TCSANOW, &newtio);

    
    uart_send = com1;
  }
