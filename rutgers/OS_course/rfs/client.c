#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#include "codes.h"
extern int errno;


int main (int argc, char **argv)
{
    int sock, fd, fd2;
    int len = 0;
    int tot = -1, i;
    char buff[24];
    char name_buff[MAX_FILES][MAX_PATH];
    
    if (argc!=3)
    {
	printf("parameters: host_name port_number\n");
	exit(-1);
    }
    
    sock = connect_to_server(argv[1], atoi(argv[2]));
    if (sock==-1)
    {
	perror("connect");
	exit(-1);
    }

    /*
      Now you can use 'sock' to read/write messages from/to the server
    */
    
    /* directory list */
    tot = client_scandir(sock, (char *)name_buff);
    printf("Returned %d files:\n", tot);

    for (i = 0; i < tot; i++) {
      printf("%s\n", name_buff[i]);
    }

    /*open remote file */
    fd = client_open(sock, "README", O_RDONLY);

    /* read its content */
    len = client_read(sock, fd, buff, sizeof(buff));
    buff[len] = '\0';
    printf("***READ %d bytes: %s\n", len, buff);

    /* try again */
    len = client_read(sock, fd, buff, sizeof(buff));
    buff[len] = '\0';
    printf("***READ2 %d bytes: %s\n", len, buff);


    /* open new file for writing */
    /*
      fd2 = client_open(sock, "write_file", O_WRONLY | O_CREAT);

    /* write to file */
    /*    strncpy(buff, "This is a file created by RFS client\nwriten for OS\n",
	  sizeof(buff));
	  len = client_write(sock, fd2, buff, strlen(buff));
	  
    /* close */
    /*    client_close(sock, fd2);
    */
    client_close(sock, fd);

    /* End */
    client_end(sock);

    return(0);
}
    

