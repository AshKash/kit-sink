#ifndef LOCAL_GROUP

#define LOCAL_GROUP 0xa
#define TOS_BCAST_ADDR (char) 0xff
#define TOS_UART_ADDR  0x7e
#define DATA_LENGTH 30

//char TOS_LOCAL_ADDRESS = 0x13;


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

#endif
