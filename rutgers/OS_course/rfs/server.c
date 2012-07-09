#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include "codes.h"

extern int errno;

void *do_chld(void *arg);

int main(int argc, char **argv) {
  int bind_port = -1;

  if (argc < 2) { 
    printf("Error! Give the port to bind to\n");
    exit(1);
  }

  sscanf(*(argv + 1), "%d", &bind_port);
  printf("Binding to port %d\n", bind_port);

  /* Call the listener/forker */
  bind_accept_fork(bind_port, do_chld);

  /* Never reached */
  return(0);
}

/* 
 * This is the routine that is executed from a new thread 
 * The thread represents one client
 */

void *do_chld(void *arg)
{
  int 	client_socket = (int) arg;
  // the socket to the server if we are the controller
  int len = 0;
  int flag = 1;

  printf("Child thread: Socket number = %d\n", client_socket);

  while (flag) {
    struct op_hdr get_hdr;

    /* Read the header*/
    printf("\t\t\tfirst read\n");
    READ_SOC(len, client_socket, &get_hdr, HDR_SIZE);
    printf("***HDR***: op: %d, p1: %d, p2: %d, p3: %d\n",
	   get_hdr.op, get_hdr.p1, get_hdr.p2, get_hdr.p3);

    /* ACT */
    switch (get_hdr.op) {

    case OP_OPEN:
      server_open(client_socket, get_hdr);
      break;

    case OP_READ:
      server_read(client_socket, get_hdr);
      break;

    case OP_WRITE:
      server_write(client_socket, get_hdr);
      break;

    case OP_SEEK:
      server_seek(client_socket, get_hdr);
      break;

    case OP_SCANDIR:
      server_scandir(client_socket, get_hdr);
      break;

    case OP_CLOSE:
      server_close(client_socket, get_hdr);
      break;

    case OP_END:
      flag = 0;
      break;

    default:

      /* ERROR! */
      /* Since we are the server, we dont know how to handle
	 errors on the client */
      get_hdr.op = -1;
      WRITE_SOC(len, client_socket, &get_hdr, HDR_SIZE);
      printf("Bad opcode: %d\n", get_hdr.op);
      break;

    }

  }
  printf("Child: Done Processing...\n"); 
  
  /* close the socket and exit this thread */
  close(client_socket);
  pthread_exit((void *)NULL);

  /* never reached */
  return(NULL);
}

/* returns directory contents of the root directory (/DFS) */
void server_scandir(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  struct op_hdr put_hdr;
  DIR *dirp = NULL;
  struct dirent *dp = NULL;
  int next = 0;
  char file_list[MAX_FILES][MAX_PATH];

  // init
  put_hdr = get_hdr;
  printf("\tScan dir \n");

  // open the directory
  dirp = opendir(DFS);

  // Read contents of local directory and put it into array
  while(dirp && (next < MAX_FILES)) {
    if ((dp = readdir(dirp)) != NULL) {
      if (strcmp(dp->d_name, ".") != 0 
	  && strcmp(dp->d_name, "..") != 0) {
	strcpy(file_list[next++], dp->d_name);
      }
    } else {
      closedir(dirp);
      break;
    }
  }

  // Send the size of array (number of files)
  put_hdr.p1 = next;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  // Send the array
  WRITE_SOC(len, client_socket, file_list, MAX_PATH * next);

  printf("Directory scan complete\n");
}

/*
 * Called when server got OP_OPEN
 * The server opens the file and passes the fd back to client
 */
void server_open(int client_socket, struct op_hdr get_hdr) {
  int len = -1;
  int fd = -1;  // FD
  int mode = -1;
  int name_len = 0;
  struct op_hdr put_hdr;
  char path_buff[4096];

  // init
  put_hdr = get_hdr;
  mode = get_hdr.p1;
  name_len = get_hdr.p2;    // Size of file name
  printf("\tname_len: %d, mode: %d\n", name_len, mode);

  /* Generate path prefix */
  strcpy(path_buff, DFS);

  /* Read the actual data (name of the file) */
  READ_ALL(len, client_socket, path_buff + strlen(DFS), name_len);
  path_buff[name_len + strlen(DFS)] = '\0';
  printf("\tfilename: %s\n", path_buff);

  /* Open the requested file */
  put_hdr.op = OP_OPEN;
  if ((fd = open(path_buff, mode,(mode_t)0644)) < 0) {
    perror("File open failed");
    put_hdr.op = ERR_OPEN;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }

  /* Reply with the result */
  put_hdr.p1 = fd;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  printf("Opened %s, FD: %d\n", path_buff, fd);

}


/*
 * Called on OP_READ request
 * The server reads the given fd and then transmits it back
 */
void server_read(int client_socket, struct op_hdr get_hdr)
{
  int len = -1, len2 = -1;
  int fd = -1;  // FD
  int client_size = -1;    // buffer on client side
  struct op_hdr put_hdr;
  void *ptr = NULL;

  // init
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  client_size = get_hdr.p2;
  printf("\tRead on FD: %d\n", fd);

  /* Read the file */
  MALLOC(ptr, client_size);
  if ((len2 = read(fd, ptr, client_size)) < 0) {
    free(ptr);
    perror("server failed to read file");
    put_hdr.op = ERR_READ;
    put_hdr.p1 = errno;

    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }
      
  /* Reply */
  put_hdr.p1 = len2;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /* Send the data */
  WRITE_SOC(len, client_socket, ptr, put_hdr.p1);
  free(ptr);

  printf("\tSent file of size: %d\n", put_hdr.p1);

}

/*
 * Called on OP_WRITE request
 */
void server_write(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  int fd = -1;  // FD
  void *ptr = NULL;
  int file_size = -1;
  struct op_hdr put_hdr;

  // init
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  file_size = get_hdr.p2;
  printf("\tWrite on FD: %d\n", fd);

  /* write the file to disk */
  MALLOC(ptr, file_size);
  READ_ALL(len, client_socket, ptr, file_size);
  if ((len = write(fd, ptr, file_size)) != file_size) {
    perror("Server failed to write file");
    put_hdr.op = ERR_WRITE;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    free(ptr);
    return;
  }
  free(ptr);

  /* Reply */
  put_hdr.p1 = len;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  printf("\tWrote file of size: %d\n", file_size);

}

/*
 * Called on OP_SEEK request
 */
void server_seek(int client_socket, struct op_hdr get_hdr)
{
  int fd = -1;  // FD
  off_t offset = -1;
  int whence = -1;
  int len = -1;
  struct op_hdr put_hdr;

  // init
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  offset = get_hdr.p2;
  whence = get_hdr.p3;

  printf("\tSeek on FD: %d, offset: %d, whence: %d\n",
	 fd, (int)offset, (int)whence);

  /* seek the given fd */
  if ((put_hdr.p1 = lseek(fd, offset, whence)) < 0) {
    perror("Server failed to seek");
    put_hdr.op = ERR_SEEK;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }

  /* Reply to client */
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  printf("seek on %d returned %d\n", fd, put_hdr.p1);
}

/*
 * Called on OP_CLOSE request
 */
void server_close(int client_socket, struct op_hdr get_hdr)
{
  struct op_hdr put_hdr;
  int fd = -1;
  int len = -1;

  // init
  put_hdr = get_hdr;
  fd = get_hdr.p1;

  /* close the fd */
  if ((put_hdr.p1 = close(fd) < 0)) {
    perror("File close failed");
    put_hdr.op = ERR_CLOSE;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }

  /* reply to client */
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  printf("Closed file FD: %d\n", fd);

}

