// file to test out the different funcs in various files

/*
  $Log$
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TBF_MAIN
#include "mapping.h"
#include "tbf_hdr.h"
extern struct neigh_table n_list[MAX_NEIGH];
extern int total_neigh;
struct assoc_table assoc_list[MAX_NEIGH];

void usage(void)
{

  printf ("Enter test option:\n");
  printf("\trpn\n");
  printf("\tneigh\n");
  printf("\ttraj\n");
  printf("\tpolicy\n");

  exit(1);
}

// prints n_list
void print_neigh_table()
{
  int i;
  for(i=0; i< total_neigh; i++)
    printf("%d, (%f, %f), %d, %d\n", n_list[i].id, n_list[i].x,
	  n_list[i].y, n_list[i].dirty, n_list[i].clock);

}

// prints the assoc_table given
void print_assoc_table(struct assoc_table *at)
{
  int i;
  for(i=0; i< total_neigh; i++ ){
    printf("%d->[%lf=(%lf, %lf)], %lf\n", at[i].id, at[i].t, at[i].x, at[i].y, at[i].dist);
  }
}


// test the policy alogs
int test_policy()
{
  char data[30];
  struct trajectory my_traj;
  struct tbf_hdr *ptbf_hdr = (struct tbf_hdr *) data;
  int res;
  moteid_t next_node;

  // this node is at 10,10 and has the packet
  // it is at t=0 (or it is the originator)
  // it has 4
  update_neigh(1, 15.0, 15.0);
  update_neigh(2, 45.0, 11.0);
  update_neigh(3, 75.0, 5.0);
  update_neigh(4, 58.0, 15.0);

  // create the packet
  if (dump_traj("t+10", "2*sin(t/100)+10", ptbf_hdr, 30) < 0) {
    printf("Bad expression\n");
    exit(1);
  }

  // read off the packet
  get_traj(ptbf_hdr, &my_traj);
  printf("Verifying: \n");
  symb_print(my_traj.x_traj, my_traj.x_byte);
  symb_print(my_traj.y_traj, my_traj.y_byte);

  // run the policy
  ptbf_hdr->t = 0;
  printf("running the max_progress policy\n");
  res = policy_max_progress(ptbf_hdr, &next_node);
  if (res == PKT_DROP) printf("packet was dropped\n");
  else if (res == PKT_FWD) printf("packet fwded\n");
  else printf("%d is unknown\n", res);
  printf ("next node is %d, t val is %f\n", next_node, ptbf_hdr->t);
  print_assoc_table(assoc_list);

  // run the min dist policy
  ptbf_hdr->t =0;
  printf("running the min_dist policy\n");
  res = policy_min_distance(ptbf_hdr, &next_node);
  if (res == PKT_DROP) printf("packet was dropped\n");
  else if (res == PKT_FWD) printf("packet fwded\n");
  else printf("%d is unknown\n", res);
  printf ("next node is %d, t val is %f\n", next_node, ptbf_hdr->t);
  print_assoc_table(assoc_list);

  return 1;
}


void tmp_me_not(struct tbf_hdr *ptbf)
{
  printf("got pkt\n");
}

// test the traj algos
int test_traj()
{
  char data[30];    // this mimics the 30 byte array found in tos_msg
  char inf_x[64], inf_y[64];
  struct tbf_hdr *ptbf_hdr = (struct tbf_hdr *)data;
  struct trajectory my_traj;
  int err;
  double t_out;
  double x, y;

  // get a circle centered at (10,10) with radius 10
  strcpy(inf_x, "10*sin(t)+10");
  strcpy(inf_y, "10*cos(t)+10");

  // convert to postfix
  if (dump_traj(inf_x, inf_y, ptbf_hdr, 30) < 0) {
    printf("Could not convert expression given");
    exit(1);
  }

  // get the info in the trajectory struct
  get_traj(ptbf_hdr, &my_traj);
  printf("Verification:\n");
  symb_print(my_traj.x_traj, my_traj.x_byte);
  symb_print(my_traj.y_traj, my_traj.y_byte);

  // find the closest point on the curve from (0,0)
  err = find_closest(&my_traj, M_PI, M_PI*3/2, 0.0, 0.0, &t_out);
  rpncalc(my_traj.x_traj, my_traj.x_byte, t_out, &x);
  rpncalc(my_traj.y_traj, my_traj.y_byte, t_out, &y);
  printf("t: %lf->(%lf, %lf)\n", t_out, x, y);

  // tmp stuff
  tbf_tos_register(10, tmp_me_not);
  tbf_tos_list[10](ptbf_hdr);
  return 1;
}

// test if the neighbour list works ok
int test_neigh()
{
  int i;
  int total;

  // fill up the neigh table
  printf("fill n_list table\n");
  for (i = 0; i < MAX_NEIGH; i++) {
    update_neigh(i, (double)rand(), (double)rand());
  }

  printf("added total %d\n", total_neigh);
  print_neigh_table();

  // find a given mote id
  i = find_neigh_id(10);
  printf ("found id 10 at index: %d\n", i);

  // find a given mote id by coords
  i = find_neigh_coord(1914544896.0, 859484416.0);
  printf("found (1914544896.0, 859484416.0) at: %d\n", i);

  // remove dead nodes
  printf("remove dead nodes\n");
  remove_dead_nodes();
  print_neigh_table();

  // remove neighs
  printf("remove all neigh\n");
  for (i=0, total=total_neigh; i< total; i++) {
    remove_neigh(0);
  }
  print_neigh_table();


  return 1;
}

// test rpn stuff
int test_rpn()
{
  char infix_str[200];
  char postfix_str[200];
  int k, n;
  double result;

  while(1) {
    printf("Enter infix: ");
    if (scanf("%s%n", infix_str, &n) < 1) break;
    infix_str[n] = '\0';
    
    k = intopost(infix_str, postfix_str);
    if(k < 0) {
      printf("Error in expression, please re-enter\n");
      continue;
    }
    //bin_print(postfix_str, k);
    symb_print(postfix_str, k);

    n = rpncalc(postfix_str, k, 5.0, &result);
    if (n < 0) {
      printf("Expression was bad\n");
      continue;
    }
    printf("value is %lf\n", result);
  }
  return 1;
}

int main(int argc, char **argv)
{
  char *opt = argv[1];

  if (argc < 2 ) usage();

  if (strcasecmp(opt, "neigh") == 0) test_neigh();
  else if (strcasecmp(opt, "rpn") == 0) test_rpn();
  else if (strcasecmp(opt, "traj") == 0) test_traj();
  else if(strcasecmp(opt, "policy") == 0) test_policy();

  else usage();
  return 0;
}
