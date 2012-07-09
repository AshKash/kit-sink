#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "timing.h"
#include <sys/time.h>

#define LOCAL_GROUP 0xa
#define TOS_BCAST_ADDR (char) 0xff
#define TOS_UART_ADDR  0x7e
#define DATA_LENGTH 30
#ifndef LOCAL_GROUP
#error "You are using communication modules. Please define LOCAL_GROUP id (a hex numer, in range 0x00-0xff)"
#endif
char TOS_LOCAL_ADDRESS = 0x13;

struct MSG_VALS{
char addr;
char type;
unsigned char group;
char data[DATA_LENGTH];
//short crc;
//int strength;
};

#define TOS_Msg struct MSG_VALS
#define TOS_MsgPtr struct MSG_VALS*

#define BAUDRATE B19200

// inverse freq of error (how much delay before xmit delay is decreased)
#define ERR_MAX 0xf
#define ERR_CNT (err_cnt & ERR_MAX)
#define DELAY_AVG (280 * 1000)
#define DELAY_MIN (260 * 1000)

int
main(int argc, char **argv)
{
  struct termios oldtio, newtio;
  TOS_Msg msg;
  int com1;
  int res;
  long int delay = DELAY_AVG;
  char err_cnt = ERR_MAX;

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


  // Fill the msg
  msg.addr = TOS_BCAST_ADDR;
  msg.group = LOCAL_GROUP;
  strcpy(&msg.data, "This is one messy thing\r\n");
  
  // send it
  do {
	  TOS_Msg error;
	  
	  // try to read a pkt and see if there was any error, if so, wait
	  if ((res = read(com1, &error, sizeof(error))) == sizeof(error)) {
		  printf("got err %d", (int)error.type);
		  // see if this is some error msg
		  if (error.type == -1) {
			  // wait for some time
			  printf(" ... waiting\n");
			  err_cnt += ERR_MAX;
			  delay += delay / 10;
			  //usleep(delay);
		  } else {
			  // oops this pkt must be passed to the handler !
			  // fill  here later
		  }
	  }

	  delay = delay - (delay > DELAY_MIN) * (!(err_cnt % ERR_MAX)) * (err_cnt <= 0) * delay / 10;
	  err_cnt--;

	  if (write(com1, &msg, sizeof(msg)) < 0) {
		perror("While trying to write to UART");
		exit(1);
	  }
	  
	  usleep(delay);
	  printf("Sent pkt, delay = %ld, err = %d\n", delay, err_cnt);
  } while (1);
  
}

