#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mote-gw.h"

extern int errno;


TOS_Msg * am_handler_4(TOS_Msg *am_pkt)
{

  /* Display count on screen */
  printf("Count: %d\n", am_pkt->data[0]);


  /* Packet must now be dropped */
  return NULL;
}

int main (int argc, char **argv)
{

  int server_socket = client_init(argc, argv);
  TOS_Msg am;
  char i;

  if (server_socket < 0) {
    printf("Failed to connect to server\n");
    exit(1);
  }


  /* Register the am handler 4 */
  reg_am_type(4, am_handler_4);

  /* form the packet */
  am.addr = TOS_BCAST_ADDR;
  am.type = (char) 4;    /* INT_TO_RFM */
  am.group = LOCAL_GROUP;

  while (1) {
    am.data[0] = i++;
    send_tos_msg(server_socket, &am);
    // recv_tos_msg(server_socket, &am);
    //i++;
       printf("send pkt %d\n", am.data[0]);
  }



  /* Exit */
  return(0);
}
    
