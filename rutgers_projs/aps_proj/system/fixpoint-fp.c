
    /*
     *  A non-optimized fixpoint pkg, with some overflow checking.  
     */ 
#ifdef FULLPC
#include <stdio.h>
#include <stdlib.h>
#endif
    
#include "fixpoint-fp.h"
int            fp_verbose;
int            fp_error = 0;	/* will contain error indicators for
				 * fp_multiply */
int            fp_print_error = 1;	/* if 1, print overflow errors */
fixpoint fp_multiply(fixpoint x, fixpoint y) 
{
    return (x * y);
}
fixpoint fp_div(fixpoint x, fixpoint y) 
{
    return x / y;
}
fixpoint fp_sqrt(fixpoint x) 
{
    return sqrt(x);
}
fixpoint fp_int_fix(int16 x) 
{
    return (fixpoint) x;
}
fixpoint fp_abs(fixpoint x) 
{
    return abs(x);
}
void
fp_print(fixpoint x) 
{
    
#ifdef FULLPC 
	printf("%f", x);
    
#endif
} 
