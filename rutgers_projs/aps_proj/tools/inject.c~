#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "timing.h"
#include <sys/time.h>
struct timespec delay, delay1;
struct timeval tv;



#define BAUDRATE B19200
//#define BAUDRATE B9600

#define NPGMS 5
char msg0[30] = {0x05,	/* base mote address */
		   7,		/* upload code capsule */
		   3,		/* length */
		   0x42,	/* pushc 000010 */
		   0x08,	/* putled (on g) */
		   0x0};	/* halt */

  char msg1[30] = {0x05,	/* base mote address */
		   7,		/* upload code capsule */
		   3,		/* length */
		   0x66,	/* pushc 100110 */
		   0x08,	/* putled (toggle gy) */
		   0x0};	/* halt */
				/* get sensor 0, send data */
  char msg2[30] = {0x05,	/* base mote address */
		   7,		/* upload code capsule */
		   4,		/* length */
		   0x40,	/* pushc 00*/
		   0x0e,	/* get sensor data */
		   0x0f,	/* send data */
		   0x0};	/* halt */

  char msg3[30] = {0x05,	/* base mote address */
		   7,		/* upload code capsule */
		   3,		/* length */
		   0x5f,	/* pushc 101111/
		   0x0f,	/* send data */
		   0x0};	/* halt */

  char msg4[30] = {0x05,	/* base mote address */
		   7,		/* upload code capsule */
		   7,		/* length */
		   0x41,	/* pushc 1 */
		   0x42,	/* pushc 2 */
		   0x43,	/* pushc 1 */
		   0x01,	/* add */
		   0x01,        /* add */
		   0x0f,	/* send data */
		   0x0};	/* halt */

char *pgms[NPGMS] = {msg0, msg1, msg2, msg3, msg4};
int  response[NPGMS] = {1,1,1024,2, 1};


char pgm[30];

void readln(FILE *s) {
  int nchar;
  while ((nchar = getc(s)) != EOF) {
    if (nchar == '\n') return;
  }
}

int main(int argic, char ** argv) {

  int com1,i,stop,msgs=0;
  int count;
  int sofar;
  int correct;
  double start;
  char radio[255];
  unsigned int reset=0xAA0000AA;
  unsigned int rf_enable=0x80000080;
  int j;
  char text[6]="hello ";
  int output;
  int listen = 1;
  FILE *s;
  unsigned x;

  unsigned int data=0x01020304;
 
  struct termios oldtio, newtio;

  /* open com1 for read/write */ 
  com1 = open("/dev/ttyS0", O_RDWR|O_NOCTTY);
  output = open("foo.data", O_RDWR);
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


  s = fopen("test","r");
  i = 0;
  pgm[i++] = 0x05;		/* base mote addr */
  pgm[i++] = 0x07;		/* upload capsule type */
  while (fscanf(s,"%x",&x) != EOF) {
    printf("pgm[%d]=%x\n",i,x);
    pgm[i] = x;
    i++;
    readln(s);
  }


  write(com1, pgm, 30);	/* send pgm as mesg */
  printf("Sent pgm\n");
  while (listen) {
    while (readHeader(com1) < 0) {};
    count = 0;
    while(count < 29) count += read(com1, radio + count, 29 - count); 	
    if(count >= 0){
      radio[count] = 0;
      printf("data len: %d ", count);
      printf("hdlr:%x\n data: ", radio[0]);
      for(i = 1; i < 29; i ++){
	printf("%2x ", radio[i] & 0xff);
      }
      printf("\n");
      count = 0;
      /* write(output, radio, 30); */
    }
  }
}

int readFull(int file, char* buf, int length){
  int count = 0;
  int loops = 0;
  int added = 0;
  while(count < length){
    added = read(file, buf+count, length - count);
    if(added > 0){
        count += added;
        loops = 0;
    }else{
      loops++;
      if(loops > 4000 && count == 0) {
#ifdef READFULLDEBUG
	if(count > 0) printf("read abort\n");
#endif
	return -1;
      }
    }
	}
  return 1;
}

int readHeader(int file){
    char scratch[200];
    int place = 0;
    int err;
    int i;
    bzero(scratch, 200);
    err = readFull(file, scratch, 1); /* read byte into scratch */
    if(err == -1) return err;

    while((scratch[0] != (char)0x7e)){
      printf("HDR: %2x\n", (char) scratch[0]);
      place++;
      err = readFull(file, scratch, 1);	/* try again */
      if(err == -1) return err;
      if(place == 190) place = 0;
    }
    printf("hdr ok %2x: ", (char) scratch[0]);
   return 1;
}


