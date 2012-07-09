/*
 *  A non-optimized fixpoint pkg, with some overflow checking. 
 * Changes: Ashwin: fixpoint is now int32. lil bit of cleanup  
 */  
#ifdef FULLPC
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#endif
#include "hardware.h"
    
#include "fixpoint.h"
    
#define HIBITS 16
#define LOBITS 16
#define LOMASK       (~(0xffffffff << LOBITS))
#define HIMASK       ((~(0xffffffff << HIBITS)) << LOBITS)
    
#define SIGNBIT      ((uint32)1 << (HIBITS+LOBITS-1))
#define OVERFLOWMASK (SIGNBIT | ~(HIMASK | LOMASK))
    
#define fp_ten_1 0x1999          /* pow(10, -1) in fixpoint */
#define fp_ten_2 0x028f          /* pow(10, -2) in fixpoint */
#define fp_ten_3 0x0041          /* pow(10, -3) in fixpoint */
#define fp_ten_4 0x0006          /* pow(10, -4) in fixpoint */
    
#define TwosComplement(_x_)  (-(_x_))
    
#ifdef FULLPC
#define MY_ERROR(...) printf(__VA_ARGS__)
#else	/*  */
#define MY_ERROR(...) SET_YELLOW_LED_PIN();
#endif
    int32 fp_verbose;
int32 fp_error = 0;		/* will contain error indicators for
				 * fp_multiply */
int32 fp_print_error = 1;	/* if 1, print overflow errors */

    /*
     * Return the largest (smallest) number representable in this format 
     */ 
    fixpoint fp_max()
{
    return (~OVERFLOWMASK);
}
fixpoint fp_min()
{
    return (OVERFLOWMASK);
}

    /*
     * return integer part 
     */ 
    int32 fp_integer(fixpoint x) 
{
    fixpoint floor = x >> LOBITS;
    fixpoint bits = (x & LOMASK);
    if (x < 0) {
	if (!bits)
	    return (floor);
	return (floor + 1);
    }
    
    else
	return (floor);
}
fixpoint fp_abs(fixpoint x) 
{
    if (x < 0)
	return TwosComplement(x);
    
    else
	return x;
}

    /*
     * Turn a double into a fixpoint number int two's complement. 
     * Negative numbers become: ~a + 1 
     */ 
#ifdef FULLPC
    fixpoint fp_fix(double x) 
{
    int32 negative = (x < 0);
    double         i;		/* integer part */
    double         fraction;	/* fraction part, positive only */
    fixpoint p;
    if ((x < fp_integer(fp_min())) || (x >= fp_integer(fp_max())))
	MY_ERROR
	    ("sorry, %g doesn't fit in (%d hi, %d lo) bit fix point.\n",
	     x, HIBITS, LOBITS);
    x = fabs(x);
    i = floor(x);
    fraction = x - i;
    p = (((int32) i) << LOBITS) | ((int32) ((x - i) * (1 << LOBITS)));
    if (negative)
	return TwosComplement(p);
    
    else
	return (p);
}

#endif
    
    /*
     * return fraction part, in bits 
     */ 
    int32 fp_fraction(fixpoint x) 
{
    if (x < 0)
	return (TwosComplement(x) & LOMASK);
    
    else
	return (x & LOMASK);
}

    /*
     * in 2s complement, integers have all lower order bits = 0, and
     * truncating the fraction = floor. 
     */ 
#ifdef FULLPC
    fixpoint fp_floor(fixpoint x) 
{
    fixpoint answer = x & HIMASK;
    return (answer);
}

#endif
    
    /*
     * Print as a binary number 
     */ 
#ifdef FULLPC
    void
fp_printb(fixpoint x) 
{
    int32 i;
    uint32 mask;
    if (x < 0)
	x = -x;
    mask = 1 << (HIBITS + LOBITS - 1);	/* start printing hi bit */
    for (i = 0; i < HIBITS; i++) {
	if (mask & x)
	    putchar('1');
	
	else
	    putchar('0');
	mask = mask >> 1;
    }
    putchar('.');
    for (i = 0; i < LOBITS; i++) {
	if (mask & x)
	    putchar('1');
	
	else
	    putchar('0');
	mask = mask >> 1;
    }
}

#endif
    
    /*
     * return fraction part, as a double 
     */ 
#ifdef FULLPC
    double
fp_fraction_double(fixpoint x) 
{
    if (x < 0)
	return (-
		((TwosComplement(x) & LOMASK) / ((double) (1 << LOBITS))));
    
    else
	return ((x & LOMASK) / ((double) (1 << LOBITS)));
} 
#endif
    fixpoint fp_int_fix(int32 x) 
{
    int32 negative = (x < 0);
    fixpoint p;		/* fixpoint version of abs(x) */
    if ((x < fp_integer(fp_min())) || (x >= fp_integer(fp_max())))
	MY_ERROR
	    ("sorry, %d doesn't fit in (%d hi, %d lo) bit fix point.\n",
	     x, HIBITS, LOBITS);
    x = fp_abs(x);
    p = ((x) << LOBITS);
    if (negative)
	return TwosComplement(p);
    
    else
	return (p);
}

    /*
     * The std integer truncating divide does a floor operation, but for
     * negative numbers it does floor(abs(x/y)), so we need to case on
     * sign of x to fix that.  if x%y has no remainder, then the divide
     * is on an integer  Overflow can't happen in fp_floor_div, since
     * x/y <= x for y>=1, which must be the case since y is an integer
     * and y>0. 
     */ 
#ifdef FULLPC
    fixpoint fp_floor_div(fixpoint x, fixpoint y) 
{
    fixpoint answer;
    if (y == 0) {
	MY_ERROR("Error: fp_floor_div -- can't divide by 0!\n");
	return (fp_int_fix(0));
    }
    
    else if (y < 0) {
	MY_ERROR
	    ("Sorry, fp_floor_div(a,b) doesn't currently work for b<0.\n");
	return (fp_int_fix(0));
    }
    
    else if (x >= 0) {
	fixpoint tmp = x / y;
	answer = tmp << LOBITS;
    }
    
    else {
	fixpoint tmp = ((x / y) + (((x % y) == 0) ? 0 : -1));
	answer = tmp << LOBITS;
    }
    return (answer);
}

#endif
    fixpoint fp_multiply(fixpoint x, fixpoint y) 
{
    fixpoint answer;
    uint32 xhi, xlo, yhi, ylo, tmp;
    fp_error = 0;
    xhi = (x < 0) ? -fp_integer(x) : fp_integer(x);
    yhi = (y < 0) ? -fp_integer(y) : fp_integer(y);
    xlo = fp_fraction(x);
    ylo = fp_fraction(y);
    
	/*
	 * If xhi and yhi are both < 16 bits, this can't have machine
	 * overflow (overflow of the 32bit int). But it CAN get larger
	 * than HIBITS, or crush the sign bit, causing overflow of our
	 * fixpoint representation. 
	 */ 
	tmp = (xhi * yhi);
    if ((tmp & (((int32) OVERFLOWMASK) >> LOBITS)) != 0) {
	if (fp_print_error)
	    MY_ERROR
		("ERROR: fp_multiply() xhi*yhi = 0x%08x overflows by 0x%08x\n",
		 tmp, (tmp & (((int32) OVERFLOWMASK) >> LOBITS)));
	fp_error = -1;
	return 0X44;
    }
    answer = (tmp << LOBITS) + 
	(xhi * ylo) + (xlo * yhi) + (((xlo * ylo) >> LOBITS) & LOMASK);
    if (answer & ~(HIMASK | LOMASK)) {
	if (fp_print_error)
	    MY_ERROR
		("ERROR: fp_multiply() answer = 0x%08x, overflow 0x%08x\n",
		 answer, (answer & ~(HIMASK | LOMASK)));
	fp_error = -2;
	return (1);
    }
    
	/*
	 * Also, it can fill up the high order (sign) bit, which is
	 * overflow. 
	 */ 
	if (answer & SIGNBIT) {
	if (fp_print_error)
	    MY_ERROR
		("ERROR: fp_multiply() SIGNBIT overflow for answer = 0x%08x\n",
		 answer);
	fp_error = -3;
	return (2);
    }
    
	/*
	 * "this should never happen" 
	 */ 
	// Ashwin: Changed this from 8 to LOBITS
	if ((xlo >> LOBITS) && (ylo >> LOBITS)) {
	if (fp_print_error)
	    MY_ERROR
		("++++++++++++  ERROR -- OVERFLOW OF LOW BITS**********\n");
	fp_error = -4;
	
	    // return -1; 
	    return (3);
    }
    if (((x < 0) && (y > 0)) || ((x > 0) && (y < 0)))
	answer = -answer;
    return (answer);
}
fixpoint fp_div_fractionpart(fixpoint remainder, fixpoint y) 
{
    fixpoint ten = fp_int_fix(10), tmp, answer = 0;
    int32 ret = 0, i;
    for (i = 1; i <= 3 && remainder; i++) {
	tmp = fp_multiply(remainder, ten);
	answer = tmp / y;
	ret = 10 * ret + fp_fraction(answer);
	remainder = tmp % y;
    }
    switch (i) {
    case 2:
	answer = fp_multiply(fp_int_fix(ret), fp_ten_1);
	break;
    case 3:
	answer = fp_multiply(fp_int_fix(ret), fp_ten_2);
	break;
    case 4:
	answer = fp_multiply(fp_int_fix(ret), fp_ten_3);
	break;
    case 5:
	answer = fp_multiply(fp_int_fix(ret), fp_ten_4);
	break;
    default:
	MY_ERROR("Error in fp_div_fractionpart: i = %d\n", i);
    }
    return answer;
}
fixpoint fp_div(fixpoint x, fixpoint y) 
{
    fixpoint absx, absy;
    absx = fp_abs(x);
    absy = fp_abs(y);
    if (absy == 0) {
	MY_ERROR("Error: fp_div -- can't divide by 0!\n");
	return (fp_int_fix(0));
    }
    
    else {
	fixpoint tmp, tmp1, remainder, answer;
	tmp = absx / absy;
	tmp = tmp << LOBITS;
	remainder = absx % absy;
	if (remainder) {
	    tmp1 = fp_div_fractionpart(remainder, absy);
	    answer = tmp | tmp1;
	}
	
	else
	    answer = tmp;
	if (((x < 0) && (y > 0)) || ((x > 0) && (y < 0)))
	    answer = -answer;
	return answer;
    }
}
int32 int_sqrt(int32 x) 
{
    int32 sq_rt = 1;
    if (!x)
	return sq_rt = 0;
    
    else {
	while (1) {
	    if (sq_rt * sq_rt == x)
		break;
	    
	    else if (sq_rt * sq_rt > x) {
		sq_rt--;
		break;
	    }
	    
	    else
		sq_rt++;
	}
    }
    return sq_rt;
}
fixpoint fp_sqrt(fixpoint x) 
{
    int32 res;
    
	// printf("BITS -> %d %d\n", HIBITS, LOBITS);
	res = int_sqrt(fp_integer(x));
    res = (res << HIBITS) | (fp_fraction(x));
    return res;
}

    /*
     * Print as a hex number, but gets things a bit screwed up unless
     * HIBITS=LOBITS=16 
     */ 
#ifdef FULLPC
    void
fp_printx(fixpoint x) 
{
    printf("%4x.%04x", fp_integer(x) & LOMASK, fp_fraction(x));
} 
#endif
    
#ifdef FULLPC
    double
fpdouble(fixpoint x) 
{
    return (((double) fp_integer(x)) + fp_fraction_double(x));
} 
#endif
    
#ifdef FULLPC
    void
fp_print(fixpoint x) 
{
    printf("%.11g", fpdouble(x));
} 
#endif
    
#ifdef FULLPC
    void
printnbits() 
{
    printf
	("HIBITS = %d, LOBITS = %d, OVERFLOWMASK = 0x%8x, SIGNBIT = 0x%08x\n",
	 HIBITS, LOBITS, OVERFLOWMASK, SIGNBIT);
    printf("HIMASK = 0x%08x, LOMASK = 0x%08x\n", HIMASK, LOMASK);
    printf("overflowmask >> lobits = 0x%08x\n",
	    ((int32) OVERFLOWMASK) >> LOBITS);
} 
#endif
    
