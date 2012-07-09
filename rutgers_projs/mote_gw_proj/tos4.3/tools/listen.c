#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "timing.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define PACKET_LENGTH 33
#define BAUDRATE B19200 //the baudrate that the device is talking
#define SERIAL_DEVICE "/dev/ttyS0" //the port to use.

int input_stream;
char input_buffer[PACKET_LENGTH];

void print_usage(void);
void open_input(void);
void print_packet(void);
void read_packet(void);

int main(int argc, char ** argv) {
   print_usage();
   open_input();
   while(1){
	read_packet();
	print_packet();
   }
}

void print_usage(){
    //usage...
	printf("usage: \n");
	printf("This program reads in data from");
	printf(SERIAL_DEVICE);
	printf(" and prints it to the screen.\n");
	printf("\n");
}


void open_input(){
    /* open input_stream for read/write */ 
    struct termios newtio;
    input_stream = open(SERIAL_DEVICE, O_RDWR|O_NOCTTY);
    if (input_stream == -1)
	perror(": input_stream open fails\n make sure the user has permission to open device.\n");
    printf("input_stream opens ok\n");

    /* Serial port setting */
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;

    /* Raw output_file */
    newtio.c_oflag = 0;
    tcflush(input_stream, TCIFLUSH);
    tcsetattr(input_stream, TCSANOW, &newtio);
}


    	
void read_packet(){
	int count;
	bzero(input_buffer, PACKET_LENGTH);
	//search through to find 0x7e signifing the start of a packet
    	while(input_buffer[0] != (char)0x7e){
    		while(1 != read(input_stream, input_buffer, 1)){};
    	} 
	count = 1;
	//you have the first byte now read the rest.
	while(count < PACKET_LENGTH) {
		count += read(input_stream, input_buffer + count, PACKET_LENGTH - count); 	
	}
}

void print_packet(){
        //now print out the packet and write it to a file.
	int i;
	printf("data:");
	for(i = 0; i < PACKET_LENGTH; i ++){
		printf("%x,", input_buffer[i] & 0xff);
	}
	printf("\n");
} 
