#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include "codes.h"

#define MAX_SERVERS    128
/* Threshold for replicating files. Files with the request
   field greater than this will be replicated */
#define REPL_THRESH    5 
#define MAX_FREE       100      /* Max free value for a server */

extern int errno;

/* The table consisting of all the backend servers */
typedef struct _server_table {
  int free;                         /* Is the server free? */
  char name[256];                   /* Name of server */
}server_table;

/*
 * The table representing list of ALL files
 * This also has the entries to the server_table where the 
 * file can be found.
 */
typedef struct _file_table {
  int index[MAX_SERVERS];           /* Indexes into the server_table */
  int tot_idx;                      /* Total number of indexes */
  int request;                      /* Popularity of this file */
  char name[MAX_PATH];              /* Name of the file */
}file_table;

/* These tables are globally distributed among all servers */
server_table server_list[MAX_SERVERS];
file_table file_list[MAX_FILES];

int server_port = -1;
int total_files = 0;               /* Total number of unique files */
int total_servers = 0;             /* Total servers */

/* regulates access to server_list and total_servers */
pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

/* regulates access to file_list  and total_files*/
pthread_mutex_t f_lock = PTHREAD_MUTEX_INITIALIZER;

/* The thread to be called after accept() */
void *do_chld(void *arg);

/* Adds given hostname to the list of backend server */
void add_server(const char* server);

/* Returns the next server entry int the server_list array */
int select_backend(const char* fname);

/*
 * Replicates a given file
 * Takes the index in the server table for from and to
 */
int replicate(int f_idx, int from, int to);

int main(int argc, char **argv) {
  int bind_port = -1;
  int flag = 1;

  if (argc < 3) { 
    printf("Error! controller <port to bind to> <server's port>\n");
    exit(1);
  }

  sscanf(*(argv + 1), "%d", &bind_port);
  sscanf(*(argv + 2), "%d", &server_port);

  // get list of backend servers from user
  printf("Enter backend server or enter . to stop\n");

  while(flag) {
    char server[1024];
    scanf("%s",  server);
    if (server[0] == '.')
      flag = 0;
    else 
      add_server(server);
  }
  printf("OK controller ready\n");

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
  int backend_socket = -1;
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
      controller_open(&backend_socket, client_socket, get_hdr);
      break;

    case OP_READ:
      controller_read(&backend_socket, client_socket, get_hdr);
      break;

    case OP_WRITE:
      controller_write(&backend_socket, client_socket, get_hdr);
      break;

    case OP_SEEK:
      controller_seek(&backend_socket, client_socket, get_hdr);
      break;

    case OP_SCANDIR:
      controller_scandir(&backend_socket, client_socket, get_hdr);
      break;

    case OP_CLOSE:
      controller_close(&backend_socket, client_socket, get_hdr);
      break;

    case OP_END:
      flag = 0;
      WRITE_SOC(len, backend_socket, &get_hdr, HDR_SIZE);
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

/* Adds given hostname to the list of backend server */
void add_server(const char* server)
{
  int backend_socket = -1;
  int total = -1;
  int tot_serv = -1;    // local copy of total_servers
  int i, j, flag = 0;
  char dir_buff[MAX_FILES][MAX_PATH];

  /* Connect to the server and get a list of files */
  if ((backend_socket = connect_to_server(server, server_port)) < 0) {
    perror("Error!");
    printf("Server not added to list\n");
    return;
  }

  /* list remote directory */
  if ((total = client_scandir(backend_socket, (char *)dir_buff)) < 0) {
    printf("Remote directory listing failed\n");
    return;
  }

   /* Add server to the server_list */
   pthread_mutex_lock(&s_lock);      // lock
   server_list[total_servers].free = MAX_FREE;    // Server is fully free
   strcpy(server_list[total_servers++].name, server);
   tot_serv = total_servers;         // copy the global
   pthread_mutex_unlock(&s_lock);    // unlock

  /* put all the files into file_list (take union) */
   pthread_mutex_lock(&f_lock);    // lock
   for (i = 0; i < total; i++) {

     /* Search for this file in the file_list */
     for (j = 0; j < total_files; j++) {
       if (strcmp(file_list[j].name, dir_buff[i]) == 0) {
	 // update the entry
	 file_list[j].request = 0;	 
	 file_list[j].index[file_list[j].tot_idx++] = tot_serv - 1;
	 flag = 1;
	 break;
       }
     }
     if (!flag) {
       // create new entry
       strcpy(file_list[total_files].name, dir_buff[i]);
       file_list[total_files].request = 0;
       file_list[total_files].tot_idx = 1;
       file_list[total_files].index[0] = tot_serv - 1;
       total_files++;
     }
     flag = 0;
   }
   pthread_mutex_unlock(&f_lock);    // unlock

   printf("***ADD_SERVER***: total files: %d, total servers %d\n", 
	  total_files, total_servers);

   // End
   client_end(backend_socket);
}

/*
 * Returns the name of the server that will process the
 * request for the client.
 * The function will modify the file_list and server_list
 */
int select_backend(const char *fname)
{
  int s_idx = -1;
  int f_idx = -1;
  int max_pair = 0;     // stores max of free+request for this file
  int max_free = 0;     // The server's index that is most free
  int i, j;


  /* Traverse all servers and change server_list[].free 
   * Also find the highest free + request pair */
  pthread_mutex_lock(&s_lock);      // lock
  max_free = 0;
  for (i = 0; i < total_servers; i++) {    
    // Increment *freeness* of all other servers (max is MAX_FREE)
    server_list[i].free = 
      (server_list[i].free < MAX_FREE)?server_list[i].free++:MAX_FREE;

    // Find server that is most free
    if (server_list[i].free > server_list[max_free].free)
      max_free = i;
  }
  pthread_mutex_unlock(&s_lock);    // unlock

  /* Locate file in the file table */
  pthread_mutex_lock(&f_lock);      // lock
  for (i = 0; i < MAX_FILES; i++) {
    if (strcmp(file_list[i].name, fname) == 0) {
      f_idx = i;
      // Update the request field
      file_list[f_idx].request++;
      break;
    }
  }

  // Traverse all servers having this file
  for (j = 0; j < file_list[f_idx].tot_idx; j++) {
    int tmp = 0;
    
    // Calc max of free+request
    if ((tmp = server_list[j].free +
	 file_list[f_idx].request) > max_pair) {
      max_pair = tmp;
      s_idx = j;         // server to connect to
    }
  }

  // See if this file needs to be replicated
  if (file_list[f_idx].request >= REPL_THRESH) {

    printf("***REPLICATE*****\n");
    printf("File: %s, server from: %s, server to: %s\n",
	   file_list[f_idx].name, server_list[s_idx].name,
	   server_list[max_free].name);
    if (replicate(f_idx, s_idx, max_free) < 0) {
      // Ignore and continue
    } else {
      s_idx = max_free;
    }
  }

  // Decrement the *freeness* of chosen server */
  server_list[s_idx].free = (server_list[s_idx].free < 2)?0:
                              (server_list[s_idx].free - 2);

  pthread_mutex_unlock(&f_lock);    // unlock    

  printf("***SELECT_BACKEND***\n");
  printf("\tRequested file: %s\n", fname);
  printf("\tChosen server: %s, server_free: %d\n\tfile_req: %d\n",
	 server_list[s_idx].name, server_list[s_idx].free,
	 file_list[f_idx].request);

  return(s_idx);

}

/*
 * Replicates a given file
 * Takes the index in the server table for from and to
 * Caller must lock the global tables (modifies the file_list)
 */
int replicate(int f_idx, int from, int to)
{
  char *f_name = file_list[f_idx].name;
  char *s_from = server_list[from].name;
  char *s_to = server_list[to].name;
  int fd_from = -1;
  int fd_to = -1;
  int sock_from = -1;
  int sock_to = -1;
  int r_len = -1, w_len = -1;
  char buff[8192];

  // See if src is same as destn
  if (from == to) {
    printf ("***NO REPLICATE*** from == to\n");
    return(-1);;
  }
  // connect to servers
  if (((sock_from = connect_to_server(s_from, server_port)) < 0 ) ||
      ((sock_to = connect_to_server(s_to, server_port)) < 0 )) {
    perror("Replication failed");
    if (sock_from >= 0) close(sock_from);
    if (sock_to >= 0) close(sock_to);
    return(-1);
  }

  // open files
  if (((fd_from = client_open(sock_from, f_name, O_RDONLY)) < 0 ) ||
      ((fd_to = client_open(sock_to, f_name, O_CREAT | O_WRONLY)) < 0 )) {
    perror("Replication failed");
    if (fd_from >= 0) client_close(sock_from, fd_from);
    client_end(sock_from);
    if(fd_to >= 0) client_close(sock_to, fd_to);
    client_end(sock_to);
    return(-2);
  }

  // read file from source server and transfer to dest server
  do {
    if ((r_len = client_read(sock_from, fd_from, buff,
			     sizeof(buff))) < 0) {
      perror("Replication failed");
      client_close(sock_from, fd_from);
      client_end(sock_from);
      client_close(sock_to, fd_to);
      client_end(sock_to);
      return(-3);
    }

    if ((w_len = client_write(sock_to, fd_to, buff,
			      r_len)) < 0 ) {
      perror("Replication failed");
      client_close(sock_from, fd_from);
      client_end(sock_from);
      client_close(sock_to, fd_to);
      client_end(sock_to);
      return(-4);
    }
  } while (r_len == sizeof(buff));    // read until eof

  // close files
  client_close(sock_from, fd_from);
  client_end(sock_from);
  client_close(sock_to, fd_to);
  client_end(sock_to);

  // update tables - add destn server to file's record
  file_list[f_idx].index[file_list[f_idx].tot_idx++] = to;

  return(0);

}


/*
 * Lists the entire list in the file_list table
 */
void controller_scandir(int *backend_socket, int client_socket,
			struct op_hdr get_hdr) {
  int len = -1;
  struct op_hdr put_hdr;
  int i = 0;
  int total = 0;        // Local copy of total_files
  char file_buff[MAX_FILES][MAX_PATH];

  // init
  put_hdr = get_hdr;

  // lock the file table
  pthread_mutex_lock(&f_lock);

  // copy list and send
  for (i = 0; i < total_files; i++) {
    strcpy(file_buff[i], file_list[i].name);
  }
  total = total_files;

  //unlock
  pthread_mutex_unlock(&f_lock);

  // reply with the result packet
  put_hdr.p1 = total;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  // send data
  WRITE_SOC(len, client_socket, file_buff, MAX_PATH * total);

  printf("Sent total of %d files\n", total);
}

/*
 * Called when controller got OP_OPEN
 * The controller passes the request to a chosen server
 */
void controller_open(int *backend_socket, int client_socket,
		     struct op_hdr get_hdr) {
  int len = -1;
  int fd = -1;  // FD
  int mode = -1;
  int name_len = 0;
  struct op_hdr put_hdr;
  int s_idx = -1;
  char path_buff[4096];

  // init
  put_hdr = get_hdr;
  mode = get_hdr.p1;
  name_len = get_hdr.p2;    // Size of file name
  printf("\tname_len: %d, mode: %d\n", name_len, mode);

  /* Read the actual data (name of the file) */
  READ_ALL(len, client_socket, path_buff, name_len);
  path_buff[name_len] = '\0';
  printf("\tfilename: %s\n", path_buff);

  // Open connection to the appropriate server
  if ((s_idx = select_backend(path_buff)) < 0) {
    printf("Requested file: %s not found\n", path_buff);
    put_hdr.op = ERR_OPEN;
    put_hdr.p1 = 0;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }

    /* Connect to server */
  if ((*backend_socket = connect_to_server(server_list[s_idx].name,
					   server_port)) < 0) {
    perror("Connection to server failed");
    put_hdr.op = ERR_OPEN;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
  }

  /* Open the requested file on the backend server */
  fd = client_open(*backend_socket, path_buff, mode);

  /* Reply with the result */
  put_hdr.op = OP_OPEN;
  put_hdr.p1 = fd;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

}


/*
 * Called on OP_READ request on the controller
 */
void controller_read(int *backend_socket, int client_socket,
		     struct op_hdr get_hdr)
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

  /* Send Read file to backend server */
  MALLOC(ptr, client_size);
  len2 = client_read(*backend_socket, fd, (void *)ptr, client_size);
      
  /* Reply */
  put_hdr.p1 = len2;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /* Send the data */
  WRITE_SOC(len, client_socket, ptr, len2);
  free(ptr);

  printf("\tSent file of size: %d\n", len2);

}

/*
 * Called on OP_WRITE request on the controller
 */
void controller_write(int *backend_socket, int client_socket,
		      struct op_hdr get_hdr)
{
  int len = -1;
  int fd = -1;  // FD
  void *ptr = NULL;
  int file_size = -1;
  struct op_hdr put_hdr;

  put_hdr = get_hdr;
  fd = get_hdr.p1;
  file_size = get_hdr.p2;
  printf("\tWrite on FD: %d\n", fd);

  /*write the file to backend */
  MALLOC(ptr, file_size);
  READ_ALL(len, client_socket, ptr, file_size);
  put_hdr.p1 = client_write(*backend_socket, fd, ptr, file_size);
  free(ptr);

  /* Reply */
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  printf("\tWrote file of size: %d\n", put_hdr.p1);

}

/*
 * Called on OP_SEEK request on the controller
 */
void controller_seek(int *backend_socket, int client_socket,
		     struct op_hdr get_hdr)
{
  int fd = -1;  // FD
  off_t offset = -1;
  int whence = -1;
  int len = -1, len2 = -1;
  struct op_hdr put_hdr;

  //init
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  offset = get_hdr.p2;
  whence = get_hdr.p3;

  printf("\tSeek on FD: %d, offset: %d, whence: %d\n",
	 fd, (int)offset, (int)whence);

  // seek on backend
  len2 = client_seek(*backend_socket, fd, offset, whence);

  put_hdr.p1 = len2;

  /* Reply */
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

}

/*
 * Called on OP_CLOSE request on the controller
 */
void controller_close(int *backend_socket, int client_socket,
		      struct op_hdr get_hdr)
{
  struct op_hdr put_hdr;
  int fd = -1;
  int len = -1;

  // init
  put_hdr = get_hdr;
  fd = get_hdr.p1;

  // close on backend
  put_hdr.p1 = client_close(*backend_socket, fd);

  printf("Closed file FD: %d\n", fd);
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

}
