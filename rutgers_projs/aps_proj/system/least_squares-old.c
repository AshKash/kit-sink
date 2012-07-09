#ifdef FULLPC
#include <stdio.h>
#include <stdlib.h>
#endif

#include "fixpoint.h"


#define MAX(a, b) ((a)>=(b) ? (a) : (b))


void
matvectmult(int n, int m, fixpoint a[][NUM_COORDINATES],
	    fixpoint c[][NUM_COORDINATES], fixpoint b[], fixpoint d[])
{
    int             i,
                    j,
                    k;
    fixpoint        s;

    /*
     * Multiply the transpose of A with A 
     */
    for (i = 0; i < m; i++)
	for (j = 0; j < m; j++) {
	    s = 0;

	    for (k = 0; k < n; k++)
		s += fp_multiply(a[k][i], a[k][j]);

	    c[i][j] = s;
	}

    /*
     * Multiply the transpose of A with b 
     */
    for (i = 0; i < m; i++) {
	s = 0;

	for (j = 0; j < n; j++)
	    s += fp_multiply(a[j][i], b[j]);

	d[i] = s;
    }
}


void
swap(fixpoint * x, fixpoint * y)
{
    fixpoint        temp;

    temp = *x;
    *x = *y;
    *y = temp;
}


fixpoint
maxnum(int m, fixpoint x[])
{
    int             k;
    fixpoint        z;

    z = MAX(fp_abs(x[0]), fp_abs(x[1]));

    for (k = 2; k < m; k++)
	z = MAX(fp_abs(x[k]), z);

    return z;
}


/*
 * Solution of Ax=b via Gauss elimination. 
 * The result is stored in parameter 'b'
 * a(N, N), b(N) 
 */
void
gausselm(int N, fixpoint a[][NUM_COORDINATES], fixpoint b[])
{
    int             i,
                    j,
                    k;
    fixpoint        u[NUM_COORDINATES],
                    m[NUM_COORDINATES - 1],
                    maxentry;

    for (i = 0; i < N - 1; i++) {
	/*
	 * determine if pivoting is needed: 
	 */

	for (k = 0; k < N - i; k++)
	    u[k] = a[k + i][i];

	maxentry = maxnum(N - i, u);

	if (fp_abs(fp_abs(a[i][i]) - maxentry) > 0) {
	    j = 0;
	    do {
		j++;
		if (j == N - i - 1)
		    break;
	    } while (fp_abs(fp_abs(a[i + j][i]) - maxentry) > 0);

	    for (k = i; k < N; k++)
		swap(&(a[i + j][k]), &(a[i][k]));

	    swap(&(b[i + j]), &(b[i]));
	}

	if (fp_abs(a[i][i]) == 0) {
#ifdef FULLPC
	    printf("Matrix singular to working accuracy.\n");
#endif
	    exit(1);
	} else {
	    for (k = 0; k < N - i - 1; k++)
		m[k] = -fp_div(a[k + i + 1][i], a[i][i]);

	    for (k = i + 1; k < N; k++) {
		b[k] += (fp_multiply(m[k - i - 1], b[i]));
		for (j = i + 1; j < N; j++)
		    a[k][j] += (fp_multiply(m[k - i - 1], a[i][j]));
	    }
	}
    }
}


void
uppertri(int n, fixpoint u[][NUM_COORDINATES], fixpoint b[])
{
    int             j,
                    k;
    fixpoint        s = 0;

    if (fp_abs(u[n - 1][n - 1]) == 0) {
#ifdef FULLPC
	printf("Matrix singular to working accuracy.\n");
#endif
	exit(1);
    }

    b[n - 1] = fp_div(b[n - 1], u[n - 1][n - 1]);

    for (k = n - 2; k >= 0; k--) {
	if (fp_abs(u[k][k]) == 0) {
#ifdef FULLPC
	    printf("Matrix singular to working accuracy.\n");
#endif
	    exit(1);
	}

	s = 0;
	for (j = k + 1; j < n; j++)
	    s += fp_multiply(u[k][j], b[j]);

	b[k] = fp_div((b[k] - s), u[k][k]);
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
least_squares(int N, int M, fixpoint a[][NUM_COORDINATES],
	      fixpoint b[], fixpoint d[])
{
    fixpoint        c[NUM_COORDINATES][NUM_COORDINATES];

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
