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
#include "server.h"

extern int errno;

/*
 ************* Globals *************
 */

/* The table consisting of all the backend servers */
typedef struct _server_table {
  int free;                         /* Is the server free? */
  int fd[MAX_FILES];                /* FDs open on this server */
  char name[MAX_S_NAME];            /* Name of server */
}server_table;

/*
 * The table representing list of ALL files
 * This also has the entries to the server_table where the 
 * file can be found.
 */
typedef struct _file_table {
  int tot_idx;                      /* Total number of server indexes */
  int request;                      /* Popularity of this file */
  int index[MAX_SERVERS];           /* Indexes into the server_table */
  char name[MAX_PATH];              /* Name of the file */
}file_table;

/* regulates access to server_list and total_servers */
pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

/* regulates access to file_list  and total_files*/
pthread_mutex_t f_lock = PTHREAD_MUTEX_INITIALIZER;

/* These tables are globally distributed among all servers */
static file_table file_list[MAX_FILES];
static server_table server_list[MAX_SERVERS];

static int server_port = -1;
static char this_host[MAX_S_NAME];
static int total_files = 0;               /* Total number of unique files */
static int total_servers = 0;             /* Total servers */

/*
 * Main program
 */
int main(int argc, char **argv) {
  int flag = 1;

  if (argc < 2) { 
    printf("Error! Give the port to bind to\n");
    exit(1);
  }
  sscanf(*(argv + 1), "%d", &server_port);

  /* get this host's name */
  if (gethostname(this_host, sizeof(this_host)) < 0) {
    perror("Gethostname");
    exit(1);
  }
  printf("This host: %s\n\n", this_host);

  /* Get list of other servers that are running */
  printf("Enter other hostnames, where server is running\nEnter . to end\n");
  while(flag && (total_servers < MAX_SERVERS)) {
    char server[MAX_S_NAME];
    scanf("%s", server);
    if (server[0] == '.')
      flag = 0;
    add_server(server);
  }

  /* Call the listener/forker */
  printf("Binding to port %d\n", server_port);
  bind_accept_fork(server_port, do_chld);

  /* Never reached */
  return(0);
}

/* Returns this host's index in the server table */
int get_this_host_idx()
{
  int i;

  for (i = 0; i < total_servers; i++) {
    if (!strcmp(server_list[i].name, this_host)) {
      return(i);
    }
  }
  /* Must never reach */
  return(-1);
}

/* Fill the local file table, fetching entries from disk */
int fill_file_list(void)
{

  int i, j, flag = 0;
  int t_files;
  int s_idx = -1;
  char dir_buff[MAX_FILES][MAX_PATH];
  
  /* get this host's entry in the server table */
  if ((s_idx = get_this_host_idx()) < 0) {
    return(-1);
  }

  /* put all local files into file_list (take union) */
  t_files = my_scandir(DFS, dir_buff);
  for (i = 0; i < t_files; i++) {
    
    /* Search for this file in the file_list */
    for (j = 0; j < total_files; j++) {
      if (strcmp(file_list[j].name, dir_buff[i]) == 0) {
	/*  update the entry */
	file_list[j].request = 0;	 
	file_list[j].index[file_list[j].tot_idx++] = s_idx;
	flag = 1;
	break;
      }
    }
    if (!flag) {
      /*  create new entry */
      strcpy(file_list[total_files].name, dir_buff[i]);
      file_list[total_files].request = 0;
      file_list[total_files].tot_idx = 1;
      file_list[total_files].index[0] = s_idx;
      total_files++;
    }
    flag = 0;
  }

  return(1);
}

/* Adds given hostname to the list of other servers */
void add_server(const char* server)
{
  int s_socket = -1;
  static int init = 0;

  /*  If we got a "." then simply fill the server table and file tables */
  /*  "." must be the first input from the user */
  if (!strcmp(".", server)) {

    /* return if init is over */ 
    if (init) return;

    /* Add this host to the server table */
    pthread_mutex_lock(&s_lock);      /*  lock  */
    strcpy(server_list[total_servers].name, this_host);
    server_list[total_servers].free = MAX_FREE;
    total_servers++;
    pthread_mutex_unlock(&s_lock);     /*  unlock */
   
    /* Fill the local file table */
    pthread_mutex_lock(&f_lock);
    if (fill_file_list() < 0) {
      printf("Error! Unable to fill the file table\n");
      exit(1);
    }
    pthread_mutex_unlock(&f_lock);

    return;
  }

  /* Connect to the server and get a list of files */
  if ((s_socket = connect_to_server(server, server_port)) < 0) {
    perror("Error!");
    printf("Server not added to list\n");
    return;
  }

  if (!init) {

    init = 1;

    /*  update the local server_list */
    pthread_mutex_lock(&s_lock);       /*  lock */
    if (update_local_st(s_socket) < 0) {
      printf("Update of local server table failed\n");
      exit(1);
    }

    /* Add this host to the server table */
    strcpy(server_list[total_servers].name, this_host);
    server_list[total_servers].free = MAX_FREE;
    total_servers++;
    pthread_mutex_unlock(&s_lock);     /*  unlock */


    /*  Update the local file_list */
    pthread_mutex_lock(&f_lock);    /*  lock */
    if (update_local_ft(s_socket) < 0) {
      printf("Failed to update local file table\n");
      exit(1);
    }
    /*  Add this server's files to the file_table */
    if (fill_file_list() < 0) {
      printf("Error! unable to fill the file table.\n");
      exit(1);
    }
    pthread_mutex_unlock(&f_lock);    /*  unlock */

  }

  /* Now we have the latest copy of both the tables
     Send this table to the remote server*/

  /*  Update the remote server table */
  pthread_mutex_lock(&s_lock);       /*  lock */
  if (update_remote_st(s_socket) < 0) {
    printf("Update of remote server table failed\n");
    pthread_mutex_unlock(&s_lock);
    exit(1);
  }
  pthread_mutex_unlock(&s_lock);     /* unlock */

  /*  Update the remote file table */
  pthread_mutex_lock(&f_lock);       /*  lock */
  if (update_remote_ft(s_socket) < 0) {
    printf("Update of remote file table failed\n");
    pthread_mutex_unlock(&f_lock);
    exit(1);
  }
  pthread_mutex_unlock(&f_lock);      /* unlock */

  printf("***ADD_SERVER***: total files: %d, total servers %d\n", 
	 total_files, total_servers);

  /*  End */
  client_end(s_socket);
}

/* 
 * This is the routine that is executed from a new thread 
 * The thread represents one client
 */

void *do_chld(void *arg)
{
  int 	client_socket = (int) arg;
  /*  the socket to the server if we are the controller */
  int len = 0;
  int flag = 1;

  printf("Child thread: Socket number = %d\n", client_socket);

  while (flag) {
    struct op_hdr get_hdr;

    /* Read the header */
    printf("\t\t\tfirst read\n");
    READ_SOC(len, client_socket, &get_hdr, HDR_SIZE);
    printf("***HDR***: op: 0x%x, p1: %d, p2: %d, p3: %d\n",
	   get_hdr.op, get_hdr.p1, get_hdr.p2, get_hdr.p3);

    /* ACT */
    switch (get_hdr.op) {

    case OP_GET_ST:
      get_st(client_socket, get_hdr);
      break;

    case OP_GET_FT:
      get_ft(client_socket, get_hdr);
      break;

    case OP_SET_ST:
      set_st(client_socket, get_hdr);
      break;

    case OP_SET_FT:
      set_ft(client_socket, get_hdr);
      break;

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
      printf("***END***\n");
      break;

    default:

      /* ERROR! */
      /* Since we are the server, we dont know how to handle
	 errors on the client */
      get_hdr.op = -1;
      WRITE_SOC(len, client_socket, &get_hdr, HDR_SIZE);
      printf("Bad opcode: 0x%x\n", get_hdr.op);
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

/*
 * Returns the name of the server that will process the
 * request for the client.
 * Caller must lock both tables
 */
int select_peer(int f_idx)
{
  int s_idx = -1;
  int max_pair = 0;     /*  stores max of free+request for this file */
  int max_free = 0;     /*  The server's index that is most free */
  int i, j;


  /* Traverse all servers and find the highest free + request pair */
  max_free = 0;
  for (i = 0; i < total_servers; i++) {    
    /*  Find server that is most free */
    if (server_list[i].free > server_list[max_free].free)
      max_free = i;
  }

  /*  Traverse all servers having this file */
  for (j = 0; j < file_list[f_idx].tot_idx; j++) {
    int tmp = 0;
    
    /*  Calc max of free+request */
    if ((tmp = server_list[j].free +
	 file_list[f_idx].request) > max_pair) {
      max_pair = tmp;
      s_idx = j;         /*  server to connect to */
    }
  }

  /* If the chosen server is the localhost */
  if (s_idx == get_this_host_idx()) {

    /* See if this file needs to be replicated
     * Only local files are replicated */
    if ((file_list[f_idx].request >= REPL_THRESH &&
	 max_free != s_idx)) {
      
      printf("***REPLICATE*****\n");
      printf("File: %s, server from: %s, server to: %s\n",
	     file_list[f_idx].name, server_list[s_idx].name,
	     server_list[max_free].name);

      if (replicate(f_idx, max_free) < 0)
	printf("Warning! Replication failed\n");
    }
  }

  printf("***SELECT_PEER***\n");
  printf("\tRequested file: %s\n", file_list[f_idx].name);
  printf("\tChosen server: %s, server_free: %d\n\tfile_req: %d\n",
	 server_list[s_idx].name, server_list[s_idx].free,
	 file_list[f_idx].request);

  return(s_idx);

}

/*
 * Increments/decrements local tables to reflect a file open
 * Must be called whenever a file is opened on localhost
 */
void on_fopen(int s_idx, int f_idx)
{
  int i;

  /*  Decrement the *freeness* of this server */
  server_list[s_idx].free = (server_list[s_idx].free < 2)?0:
    (server_list[s_idx].free - 2);
  
  /* Increment *freeness* all servers */
  for (i = 0; i < total_servers; i++) {
    server_list[i].free = 
      (server_list[i].free < MAX_FREE)?server_list[i].free++:MAX_FREE;
  }
  
  /*  Update the file's request field */
  file_list[f_idx].request++;
}

/* gets the file index in file_list, given a file name */
int get_file_idx(const char *fname)
{
  int i;

  /* Locate file in the file table */
  for (i = 0; i < total_files; i++) {
    if (strcmp(file_list[i].name, fname) == 0) {
      return(i);
    }
  }

  /* Never reached */
  return(-1);
}

/*
 * Replicates a given local file onto a remote server
 * Takes the index in the server table for "to"
 * Caller must lock the global tables (modifies the file_list)
 */
int replicate(int f_idx, int to)
{
  char *f_name = file_list[f_idx].name;
  char *s_to = server_list[to].name;
  int fd_to = -1;
  int fd_from = -1;
  int sock_to = -1;
  int r_len = -1, w_len = -1;
  char buff[8192];
  char path_buff[MAX_PATH];

  /*  connect to servers */
  if ((sock_to = connect_to_server(s_to, server_port)) < 0 ) {
    perror("Replication failed");
    return(-1);
  }

  /*  open files */
  strcpy(path_buff, DFS);
  strcpy(path_buff + strlen(DFS), f_name);
  if ((fd_from = open(path_buff, O_RDONLY)) < 0) {
    perror("Replication failed");
    return(-2);
  }
  if ((fd_to = client_open(sock_to, f_name,
			   O_CREAT | O_WRONLY | O_REPLICATE)) < 0 ) {
    perror("Replication failed");
    client_end(sock_to);
    return(-2);
  }

  /*  read file from source and transfer to dest server */
  do {
    if ((r_len = read(fd_from, buff, sizeof(buff))) < 0) {
      perror("Replication failed");
      close(fd_from);
      client_close(sock_to, fd_to);
      client_end(sock_to);
      return(-3);
    }

    if ((w_len = client_write(sock_to, fd_to, buff,
			      r_len)) < 0 ) {
      perror("Replication failed");
      close(fd_from);
      client_close(sock_to, fd_to);
      client_end(sock_to);
      return(-4);
    }
  } while (r_len == sizeof(buff));    /*  read until eof */

  /*  close files */
  close(fd_from);
  client_close(sock_to, fd_to);
  client_end(sock_to);

  /*  update tables - add destn server to file's record */
  file_list[f_idx].index[file_list[f_idx].tot_idx++] = to;

  return(0);

}

/*
 * Updates a remote server table with the local server's server_list 
 * caller locks all tables
 */
int update_remote_st(int s_socket)
{
  struct op_hdr put_hdr, get_hdr;
  int len, i;

  put_hdr.op = OP_SET_ST;
  put_hdr.p1 = total_servers;
  WRITE_SOC(len, s_socket, &put_hdr, HDR_SIZE);

  /*  read the ack */
  READ_ALL(len, s_socket, &get_hdr, HDR_SIZE);
  if (get_hdr.op != put_hdr.op) {
    return(-1);
  }

  /* DBG write one array ele at a time */
  for (i = 0; i < total_servers; i++) {
    WRITE_SOC(len, s_socket, &server_list[i],
	      sizeof(server_table));
    //printf("update_remote_st wrote %d\n", len);
  }
  return(1);
}

/*
 * Update the local server_list from remote server
 * caller locks all tables
 */
int update_local_st(int s_socket)
{
  struct op_hdr put_hdr, get_hdr;
  int len, i;

  /* Get the remote host's server table */
  put_hdr.op = OP_GET_ST;
  WRITE_SOC(len, s_socket, &put_hdr, HDR_SIZE);

  /*  read the ack */
  READ_ALL(len, s_socket, &get_hdr, HDR_SIZE);
  if (get_hdr.op != put_hdr.op) {
    return(-1);
  }

  total_servers = get_hdr.p1;

  /* Safer to update one array element at a time */
  for (i = 0; i < total_servers; i++) {
    READ_ALL(len, s_socket, &server_list[i],
	     sizeof(server_table));
  }
 
  return(1); 
}

/*
 * Update the local file_list from remote server
 * caller locks all tables
 */
int update_local_ft(int s_socket)
{
  struct op_hdr put_hdr, get_hdr;
  int len, i;

    /* Get the remote host's file table */
    put_hdr.op = OP_GET_FT;
    WRITE_SOC(len, s_socket, &put_hdr, HDR_SIZE);

   /*  Read the ack */
    READ_ALL(len, s_socket, &get_hdr, HDR_SIZE);
    if(get_hdr.op != put_hdr.op) {
      return(-1);
    }

    total_files = get_hdr.p1;
    /* Safer to update one array element at a time */
    for (i = 0; i < total_files; i++) {
      READ_ALL(len, s_socket, &file_list[i],
	       sizeof(file_table));
    }
    return(1);
}

/*
 * Updates a remote file table with the local server's file_list 
 * caller locks all tables
 */
int update_remote_ft(int s_socket)
{
  struct op_hdr put_hdr, get_hdr;
  int len, i;

  put_hdr.op = OP_SET_FT;
  put_hdr.p1 = total_files;
  WRITE_SOC(len, s_socket, &put_hdr, HDR_SIZE);

  /*  read the ack */
  READ_ALL(len, s_socket, &get_hdr, HDR_SIZE);
  if (get_hdr.op != put_hdr.op) {
    return(-1);
  }

  /* DBG write one array ele at a time */
  for (i = 0; i < total_files; i++) {
    WRITE_SOC(len, s_socket, &file_list[i],
	      sizeof(file_table));
    // printf("update_remote_ft wrote %d\n", len);
  }

  return(1);
}
/* Sends the local server table */
int get_st(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  int i;
  struct op_hdr put_hdr;

  /*  return the ack */
  put_hdr.op = OP_GET_ST;
  put_hdr.p1 = total_servers;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /*  Send the array upto total_servers */
  pthread_mutex_lock(&s_lock);     /* lock */

  /* Send one array element at a time */
  for (i = 0; i < total_servers; i++) {
    WRITE_SOC(len, client_socket, &server_list[i],
	      sizeof(server_table));
    // printf("get_st wrote: %d\n", len);
  }
  pthread_mutex_unlock(&s_lock);   /* unlock */

  return(1);

}

/* Sends the local File table */
int get_ft(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  int i;
  struct op_hdr put_hdr;

  /*  return the ack */
  put_hdr.op = OP_GET_FT;
  put_hdr.p1 = total_files;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /*  Send the array upto total_files */
  pthread_mutex_lock(&f_lock);     /* lock */

  /* Write one struct element at a time */
  for (i = 0; i < total_files; i++) {
    WRITE_SOC(len, client_socket, &file_list[i],
	      sizeof(file_table));
    // printf("get_ft wrote %d\n", len);
  }
  pthread_mutex_unlock(&f_lock);    /* unlock */

  return(1);

}

/* Set this server's Server table */
int set_st(int client_socket, struct op_hdr get_hdr)
{
  int len = -1, i;
  struct op_hdr put_hdr;

  /*  Send the ack */
  put_hdr.op = OP_SET_ST;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
  
  /*  Read upto total_servers entries in the remote host */
  pthread_mutex_lock(&s_lock);        /* lock */
  total_servers = get_hdr.p1;

  /* Safer to write one array element at a time */
  for (i = 0; i < total_servers; i++) {
    READ_ALL(len, client_socket, &server_list[i],
	   sizeof(server_table));
    // printf("set_st wrote %d\n", len);
  }

  pthread_mutex_unlock(&s_lock);      /* unlock */


  return(1);

}

/* Set this server's File table */
int set_ft(int client_socket, struct op_hdr get_hdr)
{
  int len = -1, i;
  struct op_hdr put_hdr;

  /*  Send the ack */
  put_hdr.op = OP_SET_FT;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /*  Read upto total_file entries in the remote host */
  pthread_mutex_lock(&f_lock);        /* lock */
  total_files = get_hdr.p1;

  /* Safer to write one array element at a time */
  for (i = 0; i < total_files; i++) {
    READ_ALL(len, client_socket, &file_list[i],
	   sizeof(file_table));
    // printf("set_ft wrote %d, asked %d\n", len, sizeof(file_table));
  }
  pthread_mutex_unlock(&f_lock);      /* unlock */

  return(1);

}

/* Scans given directory and returns the number of entries added to array */
int my_scandir(const char *dir_name, char file_list[MAX_FILES][MAX_PATH])
{
  DIR *dirp = NULL;
  struct dirent *dp = NULL;
  int next = 0;

  /*  open the directory */
  dirp = opendir(dir_name);

  /*  Read contents of local directory and put it into array */
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

  return next;
}

/* returns directory contents of the root directory (/DFS) */
void server_scandir(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  struct op_hdr put_hdr;
  char file_list[MAX_FILES][MAX_PATH];
  int next;

  /*  init */
  put_hdr = get_hdr;
  printf("\tScan dir \n");

  /*  Send the size of array (number of files) */
  next = my_scandir(DFS, file_list);
  put_hdr.p1 = next;
  WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

  /*  Send the array */
  WRITE_SOC(len, client_socket, &file_list[0],
	    MAX_PATH * next);

  printf("Directory scan complete\n");
}

/*
 * Called when server got OP_OPEN
 * The server opens the file and passes the fd back to client
 */
void server_open(int client_socket, struct op_hdr get_hdr) {
  int len = -1;
  int fd = -1;  /*  FD */
  int mode = -1;
  int name_len = 0;
  int s_idx = -1;
  int f_idx;
  int i;
  int update_ok = 0;
  struct op_hdr put_hdr;
  char path_buff[MAX_PATH];

  /*  init */
  put_hdr = get_hdr;
  mode = get_hdr.p1;
  name_len = get_hdr.p2;    /*  Size of file name */
  printf("\tname_len: %d, mode: %d\n", name_len, mode);

  /* Generate path prefix */
  strcpy(path_buff, DFS);

  /* Read the actual data (name of the file) */
  READ_ALL(len, client_socket, path_buff + strlen(DFS), name_len);
  path_buff[name_len + strlen(DFS)] = '\0';
  if ((f_idx = get_file_idx(path_buff + strlen(DFS))) < 0) {
    errno = EACCES;
    perror("File open failed");
    put_hdr.op = ERR_OPEN;
    put_hdr.p1 = errno;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
  }
  printf("\tfilename: %s\n", path_buff);

  pthread_mutex_lock(&s_lock);       // lock
  pthread_mutex_lock(&f_lock);       // lock

  /*
   * Current server will be used if file is being replicated
   * or if the file mode has a OP_REDIRECT.
   * Send the relative path
   */
  if ((mode & O_REPLICATE) || (mode & OP_REDIRECT)) {
    int cp_mode = mode;
    s_idx = get_this_host_idx();

    /* Remove the mode bit */
    if (cp_mode & O_REPLICATE) {
      update_ok = 0;
      mode = mode & ~O_REPLICATE;
    }
    if (cp_mode & OP_REDIRECT) {
      update_ok = 1;
      mode = mode & ~OP_REDIRECT;
    }
    
  } else {

    update_ok = 1;
    /* Find the best server to get the file from */
    if ((s_idx = select_peer(f_idx)) < 0) {

      printf("Requested file: %s not found in table, new file?\n",
	     path_buff);
      /* Maybe user's mode is O_CREAT | O_WRONLY
	 assume s_idx to be local server */
      s_idx = get_this_host_idx();
    }
  }
  
  /* See if chosen server is the local host */
  if (s_idx == get_this_host_idx()) {
    
    /* Open the requested file */
    put_hdr.op = OP_OPEN;
    if ((fd = open(path_buff, mode,(mode_t)0644)) < 0) {
      perror("File open failed");
      put_hdr.op = ERR_OPEN;
      put_hdr.p1 = errno;
      WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

      pthread_mutex_unlock(&f_lock);   // unlock
      pthread_mutex_unlock(&s_lock);   // unlock
      return;
    }

    /* Add the FD to the server table */
    server_list[s_idx].fd[f_idx] = fd;

    /* Reply with the result */
    put_hdr.p1 = fd;
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);

    printf("Opened %s, FD: %d\n", path_buff, fd);
    

    /* Is it OK to send updates? */
    if (update_ok) {

      /* File is being opened on localhost, update tables */
      on_fopen(s_idx, f_idx);

      /* 
       * Both server_list and file_list have changed
       * Update all other remote server tables.
       */
      for (i = 0; i< total_servers; i++) {
	int s_sock = -1;
      
	/* Skip if localhost */
	if (i == get_this_host_idx())
	  continue;
      
	/* Connect */
	if ((s_sock = connect_to_server(server_list[i].name,
					server_port)) < 0) {
	  perror("[server_open] Connect");
	  continue;
	}
	
	/* Update remote server_list */
	if (update_remote_st(s_sock) < 0) {
	  printf("Update of server_list to %s failed\n",
		 server_list[i].name);
	  client_end(s_sock);
	  continue;
	}
	
	/* Update the remote file_list */
	if (update_remote_ft(s_sock) < 0) {
	  printf("Update of file_list to %s failed\n",
		 server_list[i].name);
	  client_end(s_sock);
	  continue;
	}
	
	/* End this session */
	client_end(s_sock);
      }
    }

  } else {
    /* Redirect */
    char *s_name = server_list[s_idx].name;
    int s_len = strlen(s_name);
    
    /* Redirect the client */
    put_hdr.op = OP_REDIRECT;
    put_hdr.p1 = s_len;        /* Length of next pkt */
    WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    
    /* Send the server to connect to */
    WRITE_SOC(len, client_socket, s_name, s_len);
    
  }

  pthread_mutex_unlock(&f_lock);   // unlock
  pthread_mutex_unlock(&s_lock);   // unlock
  
}


/*
 * Called on OP_READ request
 * The server reads the given fd and then transmits it back
 */
void server_read(int client_socket, struct op_hdr get_hdr)
{
  int len = -1, len2 = -1;
  int fd = -1;  /*  FD */
  int client_size = -1;    /*  buffer on client side */
  struct op_hdr put_hdr;

  /*  init */
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  client_size = get_hdr.p2;
  printf("\tRead on FD: %d\n", fd);

  /* Read the file */
  {
    char buff[client_size];
    if ((len2 = read(fd, buff, client_size)) < 0) {
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
    WRITE_SOC(len, client_socket, buff, put_hdr.p1);

  }
      
  printf("\tSent file of size: %d\n", put_hdr.p1);

}

/*
 * Called on OP_WRITE request
 */
void server_write(int client_socket, struct op_hdr get_hdr)
{
  int len = -1;
  int fd = -1;  /*  FD */
  int file_size = -1;
  struct op_hdr put_hdr;

  /*  init */
  put_hdr = get_hdr;
  fd = get_hdr.p1;
  file_size = get_hdr.p2;
  printf("\tWrite on FD: %d\n", fd);

  /* write the file to disk */
  {
    char buff[file_size];

    READ_ALL(len, client_socket, buff, file_size);
    if ((len = write(fd, buff, file_size)) != file_size) {
      perror("Server failed to write file");
      put_hdr.op = ERR_WRITE;
      put_hdr.p1 = errno;
      WRITE_SOC(len, client_socket, &put_hdr, HDR_SIZE);
    return;
    }
  }

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
  int fd = -1;  /*  FD */
  off_t offset = -1;
  int whence = -1;
  int len = -1;
  struct op_hdr put_hdr;

  /*  init */
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
  int s_idx;
  int f_idx;
  int i, j;
  int mode;


  /*  init */
  put_hdr = get_hdr;
  fd = get_hdr.p1;

  /* Get mode of opening */
  mode = fcntl(fd, F_GETFL);

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

  /* If the file was opened in write mode, replicate it to all servers */
  if (mode == -1) {
    perror("fcntl failed");
    return;

  } else if ((mode & O_ACCMODE) == O_WRONLY) {

    /* Search for this FD in the server_list */
    s_idx = get_this_host_idx();
    f_idx = -1;
    for (i = 0; i < total_files; i++) {
      if (server_list[s_idx].fd[i] == fd) {
	f_idx = i;
	server_list[s_idx].fd[f_idx] = 0;    /* Reset */
	break;
      }
    }
    if (f_idx < 0) return;
    
    /* Replicate this file */
    /*  Traverse all servers having this file */
    for (j = 0; j < file_list[f_idx].tot_idx; j++) {
      if (j == s_idx) {
	/* localhost */
	continue;
      }
      if (replicate(f_idx, j) < 0) {
	/* Ignore all errors */
	printf("[server_close] Warning! replication failed");
      }
    }
  }
}

