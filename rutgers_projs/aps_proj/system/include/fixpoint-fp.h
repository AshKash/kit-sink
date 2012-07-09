#ifndef FIXPOINT_H
#define FIXPOINT_H

#include <math.h>
/* Requires: LOBITS to be divisible by 2, HIBITS<=16, and LOBITS <=16. */

#define NUM_LANDMARKS_TO_USE 3
#define NUM_COORDINATES 2

#ifndef FULLPC
typedef int fixpoint;
typedef int int16;
typedef unsigned int uint16;
#else
typedef short fixpoint;
typedef short int16;
typedef unsigned short uint16;
#endif


#endif
