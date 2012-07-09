#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/uio.h>

#define LOCAL_GROUP 0x1
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

int
main(int argc, char **argv)
{
	int			sockfd;
	struct sockaddr_in	servaddr;
	TOS_Msg msg;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9876);
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	msg.addr = TOS_BCAST_ADDR;
	msg.group = LOCAL_GROUP;
	strcpy(&msg.data, "ashwin");

	while(1) {
		sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *) &servaddr,
			 sizeof(servaddr));
	}
	exit(0);
}

