/* Operation codes */
#define OP_CREAT      0x1
#define OP_OPEN       0x2
#define OP_READ       0x4
#define OP_WRITE      0x8
#define OP_SEEK       0x10
#define OP_CLOSE      0x20
#define OP_SCANDIR    0x40
#define OP_REDIRECT   0x80
#define OP_GET_FT     0x100
#define OP_GET_ST     0x200
#define OP_SET_FT     0x400
#define OP_SET_ST     0x800
#define O_REPLICATE   0x1000       /* Special file open mode */
#define OP_END        0x2000

/* Error codes */
#define ERR_CREAT        (-OP_CREAT)
#define ERR_OPEN         (-OP_OPEN)
#define ERR_READ         (-OP_READ)
#define ERR_WRITE        (-OP_WRITE)
#define ERR_SEEK         (-OP_SEEK)
#define ERR_CLOSE        (-OP_CLOSE)
#define ERR_SCANDIR      (-OP_SCANDIR)
#define ERR_REDIRECT     (-OP_REDIRECT)
#define ERR_GET_FT       (-OP_GET_FT)
#define ERR_GET_ST       (-OP_GET_ST)
#define ERR_SET_FT       (-OP_SET_FT)
#define ERR_SET_ST       (-OP_SET_ST)

/* Trailing slash is important! */
#define DFS "/tmp/os_dfs/"
#define MAX_FILES 64
#define MAX_PATH 32

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

#define WRITE_SOC(ret, socket, buff, buff_size) \
if ((ret = write(socket, (void *)(buff), buff_size)) != buff_size) { \
perror("Write on socket failed"); \
close(socket); \
pthread_exit((void *)NULL); \
}

/* ################### High Level Functions ##################### */

/*
 * Initialize the library - fill the s_name and p_num globals
 * These must be passed from the command line
 */
int rfs_init(int argc, char **argv);

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 * Returns a virtual FD that represents both the sock_fd and the file_fd
 */
int my_fopen(const char *path, int oflag);

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
ssize_t my_fread(int virtual_fd, void *buff, size_t nbyte);

/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
ssize_t my_fwrite(int virtual_fd, const void *buff, size_t nbyte);


/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
int my_fclose(int virtual_fd);


/*
 * High level routine, the user is unaware of the underlying socket
 * See the low-level routine of similar name for more details
 */
off_t my_fseek(int virtual_fd, off_t offset, int whence);


/* ################### Low Level Functions ##################### */

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
