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
#include "rfs.h"

extern int errno;
char *s_name;      /* The name of the server to contact */
int p_num;         /* The port number to connect to */

/* lock for gethostbyname */
pthread_mutex_t h_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Initialize the libreary - fill the s_name and p_num globals
 * These must be passed from the command line
 */
int rfs_init(int argc, char **argv)
{
  if (argc!=3) {
    printf("parameters: host_name port_number\n");
    exit(1);
  }
  s_name = argv[1];
  p_num = atoi(argv[2]);
  return(1);
}

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

    pthread_mutex_lock(&h_lock);
    hp = gethostbyname(hostname);
    pthread_mutex_unlock(&h_lock);
    if (!hp) {
	printf("Can't get host entry for host %s\n", hostname);
	/* herror("gethostbyname"); */
	return(-1);
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

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 * Returns a virtual FD that represents both the sock_fd and the file_fd
 */
int my_fopen(const char *path, int oflag)
{
  int s_socket = -1;
  struct op_hdr put_hdr, get_hdr;
  int len = -1;
  int r_fd, v_fd;

  /* Connect to the server (from global ptr) */
  if ((s_socket = connect_to_server(s_name, p_num)) < 0 ) {
    perror("Connect");
    return(-1);
  }

  /* Do a manual client_open() */

  /*  fill structure */
  put_hdr.op = OP_OPEN;      /*  OP code */
  put_hdr.p1 = oflag;     /*  Open mode */
  put_hdr.p2 = strlen(path); /*  Size of file name */

  WRITE_SOC(len, s_socket, &put_hdr, HDR_SIZE);

  /*  send file name */
  WRITE_SOC(len, s_socket, path, put_hdr.p2);

  /* Read response from server */
  READ_ALL(len, s_socket, &get_hdr, HDR_SIZE);
  r_fd = get_hdr.p1;       /* The remote FD */
  if (get_hdr.op == OP_REDIRECT) {
    /* OP_REDIRECT */
    int s_len = get_hdr.p1;
    char name_buff[s_len];

    /* Server redirected us to another server, next pkt has the name */
    READ_ALL(len, s_socket, name_buff, s_len);
    name_buff[s_len] = '\0';
    printf("Client redirected to %s\n", name_buff);

    /* Close old connection */
    client_end(s_socket);

    /* Connect to new server */
    if ((s_socket = connect_to_server(name_buff, p_num)) < 0 ) {
      perror("Connect");
      client_end(s_socket);
      return(-1);
    }

    /* Open file - new mode is used*/
    if ((r_fd = client_open(s_socket, path, oflag | OP_REDIRECT)) < 0) {
      client_end(s_socket);
      return(-1);
    }

  } else if(get_hdr.op != put_hdr.op) {
    /* ERR_ */
    errno = get_hdr.p1;
    perror("Client failed to open file - S");
    client_end(s_socket);
    return(-1);
  }

  /* shift SD 16 bits to left, then combine with the FD */
  v_fd = (s_socket << 16) | (r_fd & 0xFFFF);
  
  return(v_fd);     /*  return the remote fd */

}

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
ssize_t my_fread(int virtual_fd, void *buff, size_t nbyte)
{
  short r_fd;
  short s_fd;

  s_fd = virtual_fd >> 16;
  r_fd = virtual_fd & 0xFFFF;

  return(client_read(s_fd, r_fd, buff, nbyte));

}

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
ssize_t my_fwrite(int virtual_fd, const void *buff, size_t nbyte)
{
  short r_fd;
  short s_fd;

  s_fd = virtual_fd >> 16;
  r_fd = virtual_fd & 0xFFFF;

  return(client_write(s_fd, r_fd, buff, nbyte));

}

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
int my_fclose(int virtual_fd)
{
  short r_fd;
  short s_fd;
  int ret_close;

  s_fd = virtual_fd >> 16;
  r_fd = virtual_fd & 0xFFFF;

  ret_close = client_close(s_fd, r_fd);
  client_end(s_fd);

  return(ret_close);

}

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
off_t my_fseek(int virtual_fd, off_t offset, int whence)
{
  short r_fd;
  short s_fd;

  s_fd = virtual_fd >> 16;
  r_fd = virtual_fd & 0xFFFF;

  return(client_seek(s_fd, r_fd, offset, whence));

}

/* get remote list of directory */
int client_scandir(int server_sock, char *dir_buff)
{
  struct op_hdr put_hdr, get_hdr;
  int len = -1;

  /* fill structures */
  put_hdr.op = OP_SCANDIR;
  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /*  see reply */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if(get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to list directory - S");
    return(-1);
  }

  /*  get response into buff */
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

  /*  fill structure */
  put_hdr.op = OP_OPEN;      /*  OP code */
  put_hdr.p1 = oflag;     /*  Open mode */
  put_hdr.p2 = strlen(path); /*  Size of file name */

  WRITE_SOC(len, server_sock, &put_hdr, HDR_SIZE);

  /*  send file name */
  WRITE_SOC(len, server_sock, path, put_hdr.p2);

  /* Read response from server */
  READ_ALL(len, server_sock, &get_hdr, HDR_SIZE);
  if(get_hdr.op != put_hdr.op) {
    errno = get_hdr.p1;
    perror("Client failed to open file - S");
    return(-1);
  }

  return(get_hdr.p1);     /*  return the remote fd */

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
  READ_ALL(len, server_sock, (char *)buff, get_hdr.p1);

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

  /*  fill the structure */
  put_hdr.op = OP_WRITE;
  put_hdr.p1 = remote_fd;     /*  The remote fd to write to */
  put_hdr.p2 = nbyte;         /*  How many bytes we are writing */
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

  return(get_hdr.p1);    /*  bytes written by remote */
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

  /*  fill structure */
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

  return(get_hdr.p2);

}

/* THE END */
int client_end(int remote_socket)
{
  struct op_hdr put_hdr;
  int len = -1;

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
