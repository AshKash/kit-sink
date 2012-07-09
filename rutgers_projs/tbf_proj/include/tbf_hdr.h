#define TBF_START 6             //AM message handler for TBF messages 
                                //originating from base station
#define TBF_HANDLER 7           //AM handler for replies

// my coordinates (feet)
#define X_COORD LOCX
#define Y_COORD LOCY
#define COMM_RANGE 100.0       // my communication range
#define SENDER_BASED 1         // algo
#define MAX_NEIGH 32           // table size
#define MAX_CLOSEST_ITER 100   // Number of iterations to do
#define MAX_TOS_HANDLERS 256   // size of table having ptrs to handlers
#define RES_CLOSEST (0.1)      // increment resolution
#define N_CLOCK_THRESH 10      // used to remove dead neighbours
#define TBF_STACK_SIZE 30      // stack size for rpn evaluation
#define NULL (0x0)

/********** POLICY STUFF  ***********/
#define PKT_DROP 0
#define PKT_FWD  1
#define POLICY_MAX_PROGRESS 1
#define POLICY_MIN_DISTANCE 
 
typedef short moteid_t;

//total header size: 11 bytes, 6 bits
struct tbf_hdr{

  moteid_t src;  //2 byte
  moteid_t dest; //2 byte
  float t; // 4 bytes
  unsigned char ttl; //1 byte

  unsigned char tos; //1 byte
  unsigned short proc:1;
  unsigned short fwd:1;

  unsigned short policy:4;
  unsigned short x_byte:4;
  unsigned short y_byte:4;
  
};

// app layer tbf_tos handlers
#ifndef TBF_MAIN
void (*tbf_tos_list[MAX_TOS_HANDLERS])(struct tbf_hdr *ptbf);
#else
extern void (*tbf_tos_list[MAX_TOS_HANDLERS])(struct tbf_hdr *ptbf);
#endif

// to hold the neighbours
struct neigh_table {
  moteid_t id;
  float x;
  float y;
  unsigned char dirty:1;   // andrew had some idea about caching
  unsigned char clock:7;   // so we can remove dead nodes
};

// to represent the trajectory
// dont send this over packet! note the ptr
struct trajectory {
  int x_byte;
  int y_byte;
  char *x_traj;
  char *y_traj;
};

// holds the node <-> t associations
struct assoc_table {
  double t;
  double x;
  double y;
  double dist;
  moteid_t id;
};

// the stack object
struct stack {
  int stack[TBF_STACK_SIZE];
  int ptr;
  int err;
};

// for double
struct stack_dbl {
  double stack[TBF_STACK_SIZE];
  int ptr;
  int err;
};

/**********************************************************
 *  FUNCTION PROTOTYPES
 *********************************************************/
// stack stuff
void stack_init(struct stack *p);
void push(struct stack *p, int);
int pop(struct stack *p);
void clear(struct stack *p);
int stack_err(struct stack *p);

void stack_dbl_init(struct stack_dbl *p);
void push_dbl(struct stack_dbl *p, double);
double pop_dbl(struct stack_dbl *p);
void clear_dbl(struct stack_dbl *p);
int stack_dbl_err(struct stack_dbl *p);

int rpncalc(const char *, int, double, double*);
int intopost(const char *, char *);
int bin_print(const char *postfix_str, int k);
int symb_print(const char *postfix_str, int k);


// algo stuff
int update_neigh(moteid_t id, double x, double y);
inline int dump_traj(const char *inf_x, const char *inf_y,
		     struct tbf_hdr *ptbf_hdr, int data_len);
inline void get_traj(const struct tbf_hdr *ptbf_hdr, struct trajectory *traj);
int policy_max_progress(struct tbf_hdr *ptbf_hdr, moteid_t *next_node);
int policy_min_distance(struct tbf_hdr *ptbf_hdr, moteid_t *next_node);
int find_closest(const struct trajectory *traj,
		 double t_start, double t_end,
		 double x, double y,
		 double *t_out);
int find_neigh_id(moteid_t id);
int find_neigh_coord(double x, double y);
inline int remove_neigh(int index);
void remove_dead_nodes();

