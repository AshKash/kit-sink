#include "rfs.h"

#define MAX_SERVERS    4
#define MAX_S_NAME     32
/* Threshold for replicating files. Files with the request
   field greater than this will be replicated */
#define REPL_THRESH    2 
#define MAX_FREE       100      /* Max free value for a server */

/*
 ******** Function prototypes ***********
 */

/* The thread to be called after accept() */
void *do_chld(void *arg);

/* Adds given hostname to the list of backend server */
void add_server(const char* server);

/* Returns the next server entry in the server_list array */
int select_peer(int f_idx);

/*
 * Updates a remote server table with the local server's server_list 
 * caller locks all tables
 */
int update_remote_st(int s_socket);

/*
 * Update the local server_list from remote server
 * caller locks all tables
 */
int update_local_st(int s_socket);

/*
 * Update the local file_list from remote server
 * caller locks all tables
 */
int update_local_ft(int s_socket);

/*
 * Updates a remote file table with the local server's file_list 
 * caller locks all tables
 */
int update_remote_ft(int s_socket);

/* Fill the local file table, fetching entries from disk */
int fill_file_list(void);

/* Scans given directory and returns the number of entries added to array */
int my_scandir(const char *dir_name, char file_list[MAX_FILES][MAX_PATH]);

/*
 * Replicates a given file
 */
int replicate(int f_idx, int to);

/* Sends the local Server table */
int get_st(int client_socket, struct op_hdr get_hdr);

/* Sends the local File table */
int get_ft(int client_socket, struct op_hdr get_hdr);

/* Set this server's Server table */
int set_st(int client_socket, struct op_hdr get_hdr);

/* Set this server's File table */
int set_ft(int client_socket, struct op_hdr get_hdr);

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
