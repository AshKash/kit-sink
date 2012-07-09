// contains the core algorithms of TBF
// compile with intopost.c main.c and stack.c

/*
  $Log$
*/


#include <math.h>
#include "mapping.h"
#include "tbf_hdr.h"

#define MAX_DOUBLE 100000      // maximum value of a double

// neighbour list 
struct neigh_table n_list[MAX_NEIGH];
int total_neigh = 0;

inline double my_add(double a, double b) {return a+b;}
inline double my_mul(double a, double b) {return a*b;}
inline double my_div(double a, double b) {return a/b;}
inline double my_sub(double a, double b) {return a-b;}
inline double hypot(double x, double y) {  return (sqrt(x*x + y*y)); }

// fills up the trjectory structure from the received packet
// note that the pointer to the packet offset is stored in
// x_byte and y_byte.
inline void get_traj(const struct tbf_hdr *ptbf_hdr, struct trajectory *traj)
{
  // it is assumed that each packet has the tbf_hdr
  // immediately followed by the two trajectory strings
  traj->x_byte = ptbf_hdr->x_byte;
  traj->y_byte = ptbf_hdr->y_byte;
  traj->x_traj = (char *)(ptbf_hdr + 1);
  traj->y_traj = (char *)(traj->x_traj + traj->x_byte);
}

// removes the node at the given index
inline int remove_neigh(int index)
{
  struct neigh_table *n;
  if (index >= total_neigh) {
#ifdef TBF_DEBUG
    printf("n_list index too large\n");
#endif
    return -1;
  }

  n = n_list + index;
  *n = n_list[--total_neigh];
  return 1;
}

// removes inactive nodes from the table
void remove_dead_nodes()
{
  int i;
  for (i = 0; i < total_neigh; i++)
    if (n_list[i].clock > N_CLOCK_THRESH)
      remove_neigh(i--);       // i-- needed! see remove_neigh
}

// returns the index at which this node can be found
int find_neigh_id(moteid_t id) 
{
  int i;
  for (i = 0; i < total_neigh; i++)
    if (n_list[i].id == id) return i;
  return -1;
}

// returns the indexat which this node can be found
int find_neigh_coord(double x, double y)
{
  int i;
  for (i = 0; i < total_neigh; i++) 
    if (n_list[i].x == x && n_list[i].y == y)
      return i;
  return -1;
}

// will update the node's entry when a beacon is received
// returns -1 if unable to find the node
int update_neigh(moteid_t id, double x, double y)
{
  int index = find_neigh_id(id);
  int i;

  // add if this node does not already exist
  if (index < 0) {
    if (total_neigh >= MAX_NEIGH) {
#ifdef TBF_DEBUG
      printf("n_list table full\n");
#endif
      return -1;
    }
    
    n_list[total_neigh].id = id;
    n_list[total_neigh].x = x;
    n_list[total_neigh].y = y;
    n_list[total_neigh].dirty = 1;
    index = total_neigh++;
  }

  // do the clock algo to find out dead nodes
  // when a node sends out a beacon, we update it's entry
  // in the table and set its clock to 0
  // then we inccrement clocks of all other nodes
  // so at any given point in time, if the clock value of
  // some node is very high, then we need to remove it.
  if (n_list[index].x !=  x || n_list[index].y != y) {
    // dirty
    n_list[index].x = x;
    n_list[index].y = y;
    n_list[index].dirty = 1;
  }

  for (i=0; i<total_neigh; i++) 
    n_list[i].clock++;
  n_list[index].clock = 0;

  return 1;
}

// this returns the communication range of the given node
// in feet
inline double get_range() 
{
  // this approximates the actual communication range of a mote
  // we are using this primarily to calculate the timer values
  // and it need not be exact since the timer values are relative 
  // this should just be consistent among all the neighbours of a given node)

  return COMM_RANGE;

}


// this will return the current node's coordinates
inline int get_coord(double *x, double *y)
{
  // in the current implmentation with motes havin no GPS or APS
  // we return a #defined value
  *x = X_COORD;
  *y = Y_COORD;
  return 1;
}

// given a string in postfix notation and the value for t,
// evaluates the expression for the given t.
// returns a success or failure
int rpncalc(const char *str, int len, double t,
	    double *result) {
  unsigned char c;
  const char *orig = str;
  struct stack_dbl dstack;

  stack_dbl_init(&dstack);
  while (len > (str - orig)) {
    switch (c = *str++) {
    case FLOAT:
      push_dbl(&dstack, (double)*((float*)str)++);
      break;

      // i think the motes have 2byte ints
      // we definitely need to fix this
      // FIXME:
    case INT:
      push_dbl(&dstack, (double)*((short*)str)++);
      break;

    case CHAR:
      push_dbl(&dstack, (double)*((char*)str)++);
      break;

    case T: {
      double my_t = t;
      push_dbl(&dstack, my_t);
      break;
    }
    case MY_PI:
      push_dbl(&dstack, M_PI);
      break;

    default:
      if (c > START_ARG2_FUNCS) {
	// has 2 args
	double a;
	double b;
	double (*func)(double a, double b);

	a = pop_dbl(&dstack);
	b = pop_dbl(&dstack);
	// call the func
	func = f_table[c];	
	if (func != NULL) {
	  push_dbl(&dstack, func(b, a));
	} 
      } else {
	double (*func)(double a);
	double a = pop_dbl(&dstack);

	// has only one arg
	// call the function
	func = f_table[c];
	if (func != NULL) {
	  push_dbl(&dstack, func(a));
	}
      }
      break;
    } // switch
  } // while
  *result = pop_dbl(&dstack);
  if (stack_dbl_err(&dstack) < 0) {
    clear_dbl(&dstack);
    return (-1);
  }
  return 1;

} // rpncalc

// this func will give the coordinates on the traj which 
// is closest to the give point (x, y)
// this is an iteration algorithm withi starts from t_start
// finds the first local minima
// return -1 on failure
int find_closest(const struct trajectory *traj,
		 double t_start, double t_end,
		 double x, double y,
		 double *t_out)
{
  int i, flag = 0;
  int multiplier = 1;
  double min_dist = MAX_DOUBLE;
  double dist = 0;
  double traj_x, traj_y;      // points on the trajectory

  // the idea is to find the first local minima
  // then reduce the interval and continue the iteration
  // this way we get very good accuracy with fewer iterations
  // tune the multiplier value.
  // note everything works with just one loop!
  for (i = 0; i<MAX_CLOSEST_ITER; i++) {
    // find the point for given eqn and t
    if (rpncalc(traj->x_traj, traj->x_byte, t_start, &traj_x) < 0 ||
	rpncalc(traj->y_traj, traj->y_byte, t_start, &traj_y) < 0) {
#ifdef TBF_DEBUG
      printf("rpncalc failed\n");
#endif
      return -1;
    }
    
    // find distance to the point
    dist =  hypot(traj_x - x, traj_y - y);
    if (dist < min_dist) {
      min_dist = dist;
      *t_out = t_start;
      /*
      printf("Closest distance is %lf->(%lf,%lf)\n",
	     min_dist, t_start, t_end);
      */
      flag = 1;
    } else if (flag) {
      // we are past the first local minima
      t_end = t_start;
      t_start -= 2.0 * RES_CLOSEST / multiplier;
      multiplier *= 10;
      flag = 0;
    }
    
    t_start += RES_CLOSEST / multiplier;
  } // for
#ifdef TBF_DEBUG
  printf("Total iterations: %d\n", i);
#endif

  return 1;
}

// build assocations between the nodes and the t values on the curve
int build_assoc(const struct trajectory *traj, struct assoc_table *at, double t)
{
  double dist;
  double x, y;
  double t_end;
  double t_increment;
  int i;

  // init the table
  for (i=0; i<total_neigh; i++) {
    at[i].dist = MAX_DOUBLE;
    at[i].x = n_list[i].x;
    at[i].y = n_list[i].y;
    at[i].id = n_list[i].id;
  }

  t_end = t + get_range();
  t_increment = (t_end - t) / 5;       // # of pts
  for (; t < t_end; t += t_increment) {

    for (i=0; i< total_neigh; i++) {
      // find (x,y) for this t
      if (rpncalc(traj->x_traj, traj->x_byte, t, &x) < 0 ||
	  rpncalc(traj->y_traj, traj->y_byte, t, &y) < 0)
	return -1;
      // find distance from (x,y) to the node
      dist = hypot(x - at[i].x, y - at[i].y);
      if (dist < at[i].dist) {
	at[i].dist = dist;
	at[i].t = t;
	at[i].id = n_list[i].id;
      }
    } // for
  }
  return 1;
}

// implements a simple policy for forwarding/processing a packet
// the policy is to choose the neighbour that makes the most progess
// next_node must be a pointer to the TOS_Msg field, this will be modified
// along the trajectory
int policy_max_progress(struct tbf_hdr *ptbf_hdr, moteid_t *next_node)
{
  int i;
  double t;
  double t_max = 0;      // most progress on the trajectory
  double my_t;
  struct trajectory traj;
  extern struct assoc_table assoc_list[MAX_NEIGH];

  // build the associations
  get_traj(ptbf_hdr, &traj);
  my_t = ptbf_hdr->t;
  if (build_assoc(&traj, assoc_list, my_t) < 0) 
    return PKT_DROP;

  // find node making max progress
  t_max = -1;
  for (i=0; i < total_neigh; i++) {
    t = assoc_list[i].t;
    if (t > t_max) {
      t_max = t;
      ptbf_hdr->t = assoc_list[i].t;
      *next_node = assoc_list[i].id;
    }
  }

  return PKT_FWD;
}

// policy to fwd the pkt to the neighbour closest to the trajectory
int policy_min_distance(struct tbf_hdr *ptbf_hdr, moteid_t *next_node)
{
  int i;
  double dist_min = MAX_DOUBLE;      // closest to the trajectory
  double my_t;
  struct trajectory traj;
  struct assoc_table assoc_list[total_neigh];


  // build the associations
  my_t = ptbf_hdr->t;
  get_traj(ptbf_hdr, &traj);
  if (build_assoc(&traj, assoc_list, my_t) < 0)
    return -1;

  // find it
  dist_min = MAX_DOUBLE;
  for (i=0; i < total_neigh; i++) {
    if (assoc_list[i].dist < dist_min) {
      dist_min = assoc_list[i].dist;
      ptbf_hdr->t = assoc_list[i].t;
      *next_node = assoc_list[i].id;
    }
  }

  return PKT_FWD;
}

/**************** APP LAYER STUFF ***************/
// init the array
int tbf_tos_init()
{
  int i;
  for (i =0; i < MAX_TOS_HANDLERS; i++)
    tbf_tos_list[i] = NULL;

  return 1;
}

// registers the tos, so new pkts with this tos will be passed
// to the registered func
int tbf_tos_register(int tos, void (*func)(struct tbf_hdr *ptbf))
{
  tbf_tos_list[tos] = func;
  return 1;
}

// unregister
int tbf_tos_unregister(int tos, void (*func)(struct tbf_hdr *ptbf))
{
  tbf_tos_list[tos] = NULL;
  return 1;
}
    
/***************************************************************
 *
 * Phase 2 Work for receiver based approach
 *
 ***************************************************************/

#ifndef SENDER_BASED
// find the _next_ intersection point of the circle centered at (prev_x, prev_y)
// with radius r and the given trajectory
void find_xsect(const struct trajectory *traj,
		moteid_t id, double prev_x, double prev_y,
		double *xsect_x, double *xsect_y) {
  double shadow_x, shadow_y;
  double r = get_range();

  // (prev_x, prev_y) represent the locations of the previous
  // node that forwarded us the packet.
  // since the trajectory need not pass through (prev_x, prev_y)
  // we define (shadow_x, shadow_y) point. This point is the 
  // intersection of shortest line from (prev_x, prev_y) with the trajectory.
  // Once we find the (shadow_x, shadow_y) point, it makes sense to 
  // calculate the shadow_t which represents the t value at the point
  // (shadow_x, shadow_y) on the trajectory.
  // Essentially, (shadow_x, shadow_y) is the closest point on the trajectory
  // to (prev_x, prev_y).

  find_closest(traj, prev_x, prev_y, &shadow_x, &shadow_y);

  // Now we construct the equation of a circle centered at
  // (shadow_x, shadow_y) with a radius r
  // (The equation of the trajectory is in RPN form)
  // Now find all the points that intersect
  // choose the point with a HIGHER t value
  // WARNING: this will not work unless t value along the curve
  // is guaranteed to increase as we proceed from source to destination.  
}

// given the trajectory string (in RPN), we find out how long
// we will need to wait before the timeout occurs.
// it the timer expires, we need to transmit the packet
int calculate_timeval(const struct trajectory *traj, moteid_t id,
		      double prev_t, double prev_x, double prev_y)
{
  double my_x, my_y;
  double xsect_x, xsect_y;
  double range = get_range();
  int time_val;

  // find the _next_ intersection point of the circle centered at (prev_x, prev_y)
  // with radius r and the given trajectory
  find_xsect(traj, id, prev_x, prev_y, &xsect_x, &xsect_y);

  // the xsect point is the point with the most progress.
  // we calculate our distance to the intersection point
  // and return a timer value based on that.
  get_coord(&my_x, &my_y);
  time_val = range * hypot(my_x - xsect_x, my_y - xsect_y);
  return time_val;

}
#endif
