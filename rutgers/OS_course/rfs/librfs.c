#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include "codes.h"


extern int errno;

/*
 * connects to a server and returns the socket
 * takes the host name and the port
 */
int connect_to_server (const char *hostname, int port_number)
{
    int sock, ret;
    struct sockaddr_in server;
    struct hostent *hp;
    
    sock=socket(AF_INET, SOCK_STREAM, 0);
    if (sock<0) {
      perror("connect_to_server:");
      return sock;
    }

    hp = gethostbyname(hostname);
    if (!hp)
    {
	printf("Can't get host entry for host %s\n",hostname);
	//herror("gethostbyname");
	exit(-1);
    }
    
    memcpy(&server.sin_addr, hp->h_addr, sizeof(server.sin_addr));
    server.sin_port = ntohs(port_number);
    server.sin_family = AF_INET;
    
    ret=connect(sock, (struct sockaddr*)&server, sizeof(server));
    if (ret<0) {
      perror("connect_to_server");
      return ret;
    }
    
    return sock; 
}

/* get remote list of directory */
int client_scandir(int server_sock, char *dir_buff)
{
  struct op_hdr put_hdr, get_hdr;
  int len = -1;

  //fill structures
  put_hdr.op = OP_SCANDIR;
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  // see reply
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if(get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to list directory - S");
    return(-1);
  }

  // get response into buff
  READ_ALL(len, server_sock, dir_buff, MAX_PATH * get_hdr.p1);

  return(get_hdr.p1);
}

/*
 * Called when client wants to open file on a remote server
 * On successful call, returns the fd on the remote server
 */
int client_open(int server_sock, const char *path, int oflag)
{
  struct op_hdr put_hdr, get_hdr;
  int len = -1;

  // fill structure
  put_hdr.op = OP_OPEN;      // OP code
  put_hdr.p1 = oflag;     // Open mode
  put_hdr.p2 = strlen(path); // Size of file name

  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  // send file name
  WRITE_SOC(len, server_sock, path, put_hdr.p2);

  /* Read response from server */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if(get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to open file - S");
    return(-1);
  }

  printf("Open FD: %d\n", get_hdr.p1);
  return(get_hdr.p1);     // return the remote fd

}


/*
 * Called when the client wants to read and opened file
 * Returns the number of bytes read
 * if caller passed a small buffer, he has to read again
 */
ssize_t client_read(int server_sock, int remote_fd, void *buff,
		    size_t nbyte)
{
  
  struct op_hdr get_hdr, put_hdr;
  int len = -1;

  /* Fill structure */
  put_hdr.op = OP_READ;
  put_hdr.p1 = remote_fd;
  put_hdr.p2 = nbyte;
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /*read response */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if(get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to read - S");
    return(-1);
  }

  /*get data */
  READ_ALL(len, server_sock, buff, get_hdr.p1);
  printf("Client read: %s - %d bytes\n", (char *)buff, len);

  /* Return the number of bytes actually read */
  return(len);

}


/*
 * Called when the client wants to write data to a remote file
 * Returns the number of bytes actually written
 */
ssize_t client_write(int server_sock, int remote_fd,
		     const void *buff, size_t nbyte)
{
  int len = -1;
  struct op_hdr get_hdr, put_hdr;

  // fill the structure
  put_hdr.op = OP_WRITE;
  put_hdr.p1 = remote_fd;     // The remote fd to write to
  put_hdr.p2 = nbyte;         // How many bytes we are writing
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /* Go ahead and transmit the byte stream */
  WRITE_SOC(len, server_sock, buff, nbyte);

  /* read the response */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if (get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to write - S");
    return(-1);
  }

  printf("Write for %d bytes\n", get_hdr.p1);
  return(get_hdr.p1);    // bytes written by remote
}


/*
 * The client needs to close the file manually
 * files on server will not be closed even if connection
 * is closed
 */
int client_close(int server_sock, int remote_fd)
{
  struct op_hdr get_hdr, put_hdr;
  int len = -1;

  // fill structure
  put_hdr.op = OP_CLOSE;
  put_hdr.p1 = remote_fd;

  /* send */
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /* read response */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);

  if (get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to close file - S");
    return(-1);
  }

  printf("Close FD: %d\n", put_hdr.p1);
  return(get_hdr.p2);

}

/* THE END */
int client_end(int remote_socket)
{
  struct op_hdr put_hdr;
  int len = -1;

  printf("client ending session\n");
  put_hdr.op = OP_END;
  WRITE_SOC(len, remote_socket, &put_hdr, HDR_SIZE);

  close(remote_socket);
  return(0);
}


/*
 * Seek the remote file
 */
off_t client_seek(int server_sock, int remote_fd, off_t offset, int whence)
{

  struct op_hdr get_hdr, put_hdr;
  int len = -1;

  /* Fill struct*/
  put_hdr.op = OP_SEEK;
  put_hdr.p1 = remote_fd;
  put_hdr.p2 = offset;
  put_hdr.p3 = whence;
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /* read response */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if (get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to seek - S");
    return(-1);
  }

  /* return what lseek() returns */
  printf("Server returned: %d\n", get_hdr.p1);
  return(get_hdr.p1);

}

/*
 * This function binds to specified port and creates a new
 * thread when accept() returns
 * The socket accept() returns (not the ptr) is passed to the
 * function.
 * Never returns unless there is an error or server is killed.
 */
void bind_accept_fork(int bind_port, void* start_thread)
{

  int 	sockfd, newsockfd, clilen;
  struct sockaddr_in cli_addr, serv_addr;
  pthread_t chld_thr;

  /* Create socket */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket open failed");
    exit(1);
  }

  /* Fill structures */
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(bind_port);
  
  /* Bind to given port */
  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Bind failed");
    exit(1);
  }
  listen(sockfd, 5);

  for(;;){
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    printf("Server Accept()\n");

    if(newsockfd < 0) {
      perror("Accept failed");
      exit(1);
    }

    /* create a new thread to process the incomming request
       note: dont pass ptr to newsockfd! */
    if (pthread_create(&chld_thr, NULL, start_thread,
		       (void *) newsockfd) != 0) {
      perror("Thread create failed");
      exit(1);
    }
    if (pthread_detach(chld_thr)) {
      perror("Thread could not be detached");
      exit(1);
    }

    /* the server is now free to accept another socket request */
  }
  return;
}
