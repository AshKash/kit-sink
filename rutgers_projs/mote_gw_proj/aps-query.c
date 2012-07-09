#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mote-gw.h"

extern int errno;

/* the query reply handler */
#define R_HANDLER 7

TOS_Msg * get_reply(TOS_Msg *am_pkt)
{

  


  /* Packet must now be dropped */
  return NULL;
}

int main (int argc, char **argv)
{

  int server_socket = client_init(argc, argv);
  TOS_Msg am;
  char i;
  const char **myargc = argc + 3;

  if (server_socket < 0) {
    printf("Failed to connect to server\n");
    exit(1);
  }
  
  
  /* Register the am handler */
  reg_am_type(R_HANDLER, got_reply);

  /* form the packet by reading CL args */



  /* send the packet */
  send_tos_msg(server_socket, &am);
  
  printf("send pkt %d\n", am.data[0]);
}



  /* Exit */
  return(0);
}
    
