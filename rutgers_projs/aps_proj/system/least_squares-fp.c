/*
 * Changes made by Ashwin: converted all ints and unsigned ints to int16
 * and uint16 resp. these are defined in fixpoint.h 
 */

/*
 * this does not use fixpoint 
 */

#ifdef FULLPC
#include <stdio.h>
#include <stdlib.h>
#endif

#include "fixpoint-fp.h"

#define MAX(a, b) ((a)>=(b) ? (a) : (b))

void
matvectmult(int16 n, int16 m, float a[][NUM_COORDINATES],
	    float c[][NUM_COORDINATES], float b[], float d[])
{
    int16           i,
                    j,
                    k;
    float           s;

    /*
     * Multiply the transpose of A with A 
     */
    for (i = 0; i < m; i++)
	for (j = 0; j < m; j++) {
	    s = 0;

	    for (k = 0; k < n; k++)
		s += a[k][i] * a[k][j];

	    c[i][j] = s;
	}

    /*
     * Multiply the transpose of A with b 
     */
    for (i = 0; i < m; i++) {
	s = 0;

	for (j = 0; j < n; j++)
	    s += a[j][i] * b[j];

	d[i] = s;
    }
}


least_squares(int16 N, int16 M, float a[][NUM_COORDINATES],
	      float b[], float d[])
{
    float           c[NUM_COORDINATES][NUM_COORDINATES];
    float           delta,
                    x,
                    y;

    /*
     * Multiply the transpose of A with A and then with b 
     */
    /*
     * Store the result in c and d 
     */
    matvectmult(N, M, a, c, b, d);

    delta = (c[1][0] * c[0][1]) - (c[0][0] * c[1][1]);
    x = (d[1] * c[0][1] - d[0] * c[1][1]) / delta;
    y = (d[0] * c[1][0] - d[1] * c[0][0]) / delta;
    d[0] = x;
    d[1] = y;

}


/***************************** This is only for testing!!! ****************************/
#ifdef FALSE

void
swap(float *x, float *y)
{
    float           temp;

    temp = *x;
    *x = *y;
    *y = temp;
}

fixpoint
maxnum(int16 m, float x[])
{
    int16           k;
    float           z;

    z = MAX(abs(x[0]), abs(x[1]));

    for (k = 2; k < m; k++)
	z = MAX(abs(x[k]), z);

    return z;
}

/*
 * Solution of Ax=b via Gauss elimination. 
 * The result is stored in parameter 'b'
 * a(N, N), b(N) 
 */
void
gausselm(int16 N, float a[][NUM_COORDINATES], float b[])
{
    int16           i,
                    j,
                    k;
    float           u[NUM_COORDINATES],
                    m[NUM_COORDINATES - 1],
                    maxentry;

    for (i = 0; i < N - 1; i++) {
	/*
	 * determine if pivoting is needed: 
	 */

	for (k = 0; k < N - i; k++)
	    u[k] = a[k + i][i];

	maxentry = maxnum(N - i, u);

	if (abs(abs(a[i][i]) - maxentry) > 0) {
	    j = 0;
	    do {
		j++;
		if (j == N - i - 1)
		    break;
	    } while (abs(abs(a[i + j][i]) - maxentry) > 0);

	    for (k = i; k < N; k++)
		swap(&(a[i + j][k]), &(a[i][k]));

	    swap(&(b[i + j]), &(b[i]));
	}

	if (abs(a[i][i]) == 0) {
#ifdef FULLPC
	    printf("Matrix singular to working accuracy.\n");
#endif
	    exit(1);
	} else {
	    for (k = 0; k < N - i - 1; k++)
		m[k] = -a[k + i + 1][i] / a[i][i];

	    for (k = i + 1; k < N; k++) {
		b[k] += (m[k - i - 1] * b[i]);
		for (j = i + 1; j < N; j++)
		    a[k][j] += (m[k - i - 1] * a[i][j]);
	    }
	}
    }
}


void
uppertri(int16 n, float u[][NUM_COORDINATES], float b[])
{
    int16           j,
                    k;
    float           s = 0;

    if (abs(u[n - 1][n - 1]) == 0) {
#ifdef FULLPC
	printf("Matrix singular to working accuracy.\n");
#endif
	exit(1);
    }

    b[n - 1] = b[n - 1] / u[n - 1][n - 1];

    for (k = n - 2; k >= 0; k--) {
	if (abs(u[k][k]) == 0) {
#ifdef FULLPC
	    printf("Matrix singular to working accuracy.\n");
#endif
	    exit(1);
	}

	s = 0;
	for (j = k + 1; j < n; j++)
	    s += u[k][j] * b[j];

	b[k] = ((b[k] - s) / u[k][k]);
    }
}

/*
 * Least_squares method for ax=b
 * a(n, m), b(n) where n>m
 * ax=b => using least_squares => transpose(a)*ax=transpose(a)*b
 * Aply Gauss elimination to the last equation, since we want 
 * to avoid computing the invert of transpose(a)
 */
void
least_squares_old(int16 N, int16 M, float a[][NUM_COORDINATES],
		  float b[], float d[])
{
    float           c[NUM_COORDINATES][NUM_COORDINATES];

    /*
     * Multiply the transpose of A with A and then with b 
     */
    /*
     * Store the result in c and d 
     */
    matvectmult(N, M, a, c, b, d);

    gausselm(M, c, d);

    uppertri(M, c, d);
}


#define INIT 					\
  a[0][0] = (-1);			\
  a[0][1] = (1);			\
						\
  a[1][0] = (1);			\
  a[1][1] = (1);			\
						\
  a[2][0] = (0);			\
  a[2][1] = (1);			\
						\
  b[0] = (7.0);				\
  b[1] = (1.0);				\
  b[2] = (-4.0);

int
main()
{
    int             i;

    float           a[NUM_LANDMARKS_TO_USE][NUM_COORDINATES];
    float           b[NUM_LANDMARKS_TO_USE];
    float           d[NUM_COORDINATES];

    INIT;
    least_squares_old(NUM_LANDMARKS_TO_USE, NUM_COORDINATES, a, b, d);
    for (i = 0; i < NUM_COORDINATES; i++) {
	printf("d1[%d] = %f\n", i, d[i]);
    }

    INIT;
    least_squares(NUM_LANDMARKS_TO_USE, NUM_COORDINATES, a, b, d);
    for (i = 0; i < NUM_COORDINATES; i++) {
	printf("d2[%d] = %f\n", i, d[i]);
    }


}
#endif
