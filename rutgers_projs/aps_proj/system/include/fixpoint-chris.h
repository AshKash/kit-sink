#ifndef FIXPOINT_H
#define FIXPOINT_H

/* Requires: LOBITS to be divisible by 2, HIBITS<=16, and LOBITS <=16. */

#define NUM_LANDMARKS_TO_USE 3
#define NUM_COORDINATES 2


typedef int fixpoint;
typedef int int16;
typedef unsigned int uint16;

extern  fixpoint fp_max();
extern  fixpoint fp_min();
extern  int fp_integer();
extern  int fp_fraction();

extern  fixpoint fp_abs(fixpoint x);
extern  fixpoint fp_multiply(fixpoint x, fixpoint y);
extern  fixpoint fp_div(fixpoint x, fixpoint y);
extern  fixpoint fp_sqrt(fixpoint x);
extern  fixpoint fp_int_fix(int x);
#ifdef FULLPC
extern  void fp_print(fixpoint x);
#else
#define fp_print(x) ;
#endif


#endif
