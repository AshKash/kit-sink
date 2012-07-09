/* Operation codes */
#define OP_CREAT      1
#define OP_OPEN       2
#define OP_READ       3
#define OP_WRITE      4
#define OP_SEEK       5
#define OP_CLOSE      6
#define OP_SCANDIR    7
#define OP_REPLICATE  8
#define OP_END        16

/* Error codes */
#define ERR_CREAT        (-OP_CREAT)
#define ERR_OPEN         (-OP_OPEN)
#define ERR_READ         (-OP_READ)
#define ERR_WRITE        (-OP_WRITE)
#define ERR_SEEK         (-OP_SEEK)
#define ERR_CLOSE        (-OP_CLOSE)
#define ERR_SCANDIR      (-OP_SCANDIR)
#define ERR_REPLICATE    (-OP_REPLICATE)

/* Trailing slash is important! */
#define DFS "/tmp/os_dfs/"
#define MAX_FILES 1024
#define MAX_PATH 256

struct op_hdr {
  int op;
  int p1;
  int p2;
  int p3;
};

#define HDR_SIZE sizeof(struct op_hdr)

/* Reads upto buff_size */
#define READ_SOC(ret, socket, buff, buff_size) \
if ((ret = read(socket, (void *)(buff), buff_size)) < 0) { \
perror("Read on socket failed"); \
close(socket); \
pthread_exit((void *)NULL); \
} 

/* Keeps reading until the needed number bytes are received */
#define READ_ALL(ret, socket, buff, buff_size) \
{ \
  int tmp = 0; \
  do { \
    READ_SOC(ret, socket, buff + tmp, buff_size); \
    tmp += ret; \
  } while(tmp < buff_size); \
}


#define MALLOC(ret, size) \
if ((ret = malloc(size)) == NULL) { \
perror("Malloc failed"); \
pthread_exit((void *)NULL); \
}

#define FREE(ptr) if (ptr != NULL) free(ptr); \
ptr = NULL;

#define WRITE_SOC(ret, socket, buff, buff_size) \
if ((ret = write(socket, (void *)(buff), buff_size)) != buff_size) { \
perror("Write on socket failed"); \
close(socket); \
pthread_exit((void *)NULL); \
}

/*
 * connects to a server and returns the socket
 * takes the host name and the port
 */
int connect_to_server (const char *hostname, int port_number);


/*
 * This function binds to specified port and creates a new
 * thread when accept() returns
 * The socket accept() returns (not the ptr) is passed to the
 * function.
 * Never returns unless there is an error or server is killed.
 */
void bind_accept_fork(int bind_port, void* start_thread);

/* returns directory contents of the root directory (/DFS) */
void server_scandir(int client_socket, struct op_hdr get_hdr);

/*
 * Called when server got OP_OPEN
 * The server opens the file and passes the fd back to client
 */
void server_open(int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_READ request
 * The server reads the given fd and then transmits it back
 */
void server_read(int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_WRITE request
 */
void server_write(int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_SEEK request
 */
void server_seek(int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_CLOSE request
 */
void server_close(int client_socket, struct op_hdr get_hdr);

/*
 * Lists the entire list in the file_list table
 */
void controller_scandir(int *backend_socket, int client_socket,
			struct op_hdr get_hdr);

/*
 * Called when controller got OP_OPEN
 * The controller passes the request to a chosen server
 */
void controller_open(int *backend_socket, int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_READ request on the controller
 */
void controller_read(int *backend_socket, int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_WRITE request on the controller
 */
void controller_write(int *backend_socket, int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_SEEK request on the controller
 */
void controller_seek(int *backend_socket, int client_socket, struct op_hdr get_hdr);

/*
 * Called on OP_CLOSE request on the controller
 */
void controller_close(int *backend_socket, int client_socket, struct op_hdr get_hdr);

/* get remote list of directory */
int client_scandir(int server_sock, char *dir_buff);

/*
 * Called when client wants to open file on a remote server
 * On successful call, returns the fd on the remote server
 */
int client_open(int server_sock, const char *path, int oflag);

/*
 * Called when the client wants to read and opened file
 * Returns the number of bytes read
 * if caller passed a small buffer, he has to read again
 */
ssize_t client_read(int server_sock, int remote_fd, void *buff,
		    size_t nbyte);


/*
 * Called when the client wants to write data to a remote file
 * Returns the number of bytes actually written
 */
ssize_t client_write(int server_sock, int remote_fd,
		     const void *buff, size_t nbyte);

/*
 * The client needs to close the file manually
 * files on server will not be closed even if connection
 * is closed
 */
int client_close(int server_sock, int remote_fd);

/*
 * Seek the remote file
 */
off_t client_seek(int server_sock, int remote_fd, off_t offset,
		  int whence);
/* THE END */
int client_end(int remote_socket);

