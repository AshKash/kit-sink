#define TOS_BCAST_ADDR (char) 0xff
#define TOS_UART_ADDR  0x7e
#define DATA_LENGTH 30
#ifndef LOCAL_GROUP
#error "You are using communication modules. Please define LOCAL_GROUP id (a hex numer, in range 0x00-0xff)"
#endif
extern const unsigned char TOS_LOCAL_ADDRESS;

struct MSG_VALS{
char addr;
char type;
unsigned char group;
char data[DATA_LENGTH];
//short crc;
short strength;
};

#define TOS_Msg struct MSG_VALS
#define TOS_MsgPtr struct MSG_VALS*

static inline TOS_MsgPtr TOS_EVENT(AM_NULL_FUNC)(TOS_MsgPtr data){return data;}
