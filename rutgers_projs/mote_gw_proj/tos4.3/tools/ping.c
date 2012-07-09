#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "timing.h"
#include <sys/time.h>
struct timespec delay, delay1;
struct timeval tv;

#define BAUDRATE B19200 //the baudrate that the device is talking
#define SERIAL_DEVICE "/dev/ttyS0" //the port to use.
#define OUTPUT_FILE "listen.output" //file to log data to
#define PACKET_LENGTH 30

int main(int argic, char ** argv) {

int input_stream,i,output_file;
int count;
char input_buffer[255];
struct termios oldtio, newtio;

    //usage...
	printf("usage:  This program sends a single packet\n");
	printf("to the serial port and then it reads from\n");

	printf(SERIAL_DEVICE);
	printf(" and prints it to the screen.\n");
	printf("The data is read in 30 byte packets and also\n");
	printf("logged to the file ");
	printf(OUTPUT_FILE);
	printf("\n");



    /* open input_stream for read/write */ 
    input_stream = open(SERIAL_DEVICE, O_RDWR|O_NOCTTY);
    output_file = open(OUTPUT_FILE, O_RDWR|O_CREAT);
    if (input_stream == -1)
	perror(": input_stream open fails\n make sure the user has permission to open device.\n");
    printf("input_stream opens ok\n");

    /* save old serial port setting */
    tcgetattr(input_stream, &oldtio);
    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;

    /* Raw output_file */
    newtio.c_oflag = 0;

    tcflush(input_stream, TCIFLUSH);
    tcsetattr(input_stream, TCSANOW, &newtio);


	{
		int i = 0;
		input_buffer[i] = 0xff; i++;
		input_buffer[i] = 0x6; i++;
		input_buffer[i] = 0x5; i++;
		input_buffer[i] = 0x0; i++;
		for(; i < 30; i ++)input_buffer[i] = i;
		write(input_stream, input_buffer, 30);	
	}

    while(1 == 1) { //loop forever;
	count = 0;
    	bzero(input_buffer, PACKET_LENGTH);

	//search throught to find 0x7e signifing the start of a packet
    	while(input_buffer[0] != (char)0x7e){
    		while(1 != read(input_stream, input_buffer, 1)){};
    	} 
	count = 1;
	//you have the first byte now read the rest.
	while(count < PACKET_LENGTH) {
		count += read(input_stream, input_buffer + count, PACKET_LENGTH - count); 	
	}
	//now print out the packet and write it to a file.
	printf("data:");
	for(i = 0; i < PACKET_LENGTH; i ++){
		printf("%x,", input_buffer[i] & 0xff);
	}
	printf("\n");
	write(output_file, input_buffer, 30);  	//write the packet to the output 
	gettimeofday(&tv, NULL);		//get the time and add it to the
	printf("%x, %x\n", tv.tv_sec, tv.tv_usec);	//output.
       	write(output_file, &tv, 8);  	
   } 
    //the correct way to close the stream.
    tcsetattr(input_stream, TCSANOW, &oldtio);
    close(input_stream);
}

