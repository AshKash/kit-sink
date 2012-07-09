#ifdef FULLPC
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#endif

#include "tos.h"
#include "AD_HOC_POS.h"

#include "least_squares.h"

#ifdef FULLPC
#define LANDMARK_FILE "landmarks.txt"
#else
#define IS_LANDMARK 0		/* If 1 then it is a landmark, otherwise
				 * not a landmark */
#define X_COORD 2
#define Y_COORD 2
#endif

#define EPSILON 0x3333		/* Epsilon = 0.2 */
#define MAX_ITERATIONS 1000	/* Exit from loop if algorithm has not */
					 /*
					  * converged in MAX_ITERATIONS 
					  */

// the clock doesnt seem to differentiate from tick1ps and tick1000ps!
// this tries to fix it
#define TICK_SP 10		// means tick once in x seconds (or clock
				// ticks)

typedef struct coords {
    fixpoint        x;
    fixpoint        y;
    fixpoint        dist;
} landmarks;


#define TOS_FRAME_TYPE AD_HOC_POS_frame
TOS_FRAME_BEGIN(AD_HOC_POS_frame)
{
    int             no_landmarks;
    char            is_landmark;
    fixpoint        estimate_x;
    fixpoint        estimate_y;
    char            aps_has_started;
    landmarks       lms[NUM_LANDMARKS_TO_USE];
    char            current_tick;	// see TICK_SP
    char            table_dirty;
}
TOS_FRAME_END(AD_HOC_POS_frame);


void
calculate_unit_vectors(fixpoint a[][NUM_COORDINATES], fixpoint b[], int N)
{
    int             i = 0;
    fixpoint        tmp_x,
                    tmp_y,
                    tmp_length;

    while ((i < VAR(no_landmarks)) && (i < N)) {
	tmp_x = VAR(estimate_x) - VAR(lms)[i].x;
	tmp_y = VAR(estimate_y) - VAR(lms)[i].y;

	tmp_length =
	    fp_sqrt(fp_multiply(tmp_x, tmp_x) + fp_multiply(tmp_y, tmp_y));
	a[i][0] = -(fp_div(tmp_x, tmp_length));
	a[i][1] = -(fp_div(tmp_y, tmp_length));
	b[i] = tmp_length - VAR(lms)[i].dist;

	i++;
    }
}


void
aps_iterations()
{
    int             iterations = 0;
    fixpoint        b[NUM_LANDMARKS_TO_USE],
                    d[NUM_COORDINATES],
                    a[NUM_LANDMARKS_TO_USE][NUM_COORDINATES];
    fixpoint        newerror = 0,
                    olderror = fp_int_fix(10000);
    fixpoint        alt_x = 0,
                    alt_y = 0;

    while (1) {
	iterations++;

	calculate_unit_vectors(a, b, NUM_LANDMARKS_TO_USE);

	least_squares(NUM_LANDMARKS_TO_USE, NUM_COORDINATES, a, b, d);

	newerror =
	    fp_sqrt(fp_multiply(d[0], d[0]) + fp_multiply(d[1], d[1]));

	if (newerror < EPSILON)
	    break;

	if (newerror > olderror) {
	    VAR(estimate_x) = alt_x;
	    VAR(estimate_y) = alt_y;
	    olderror = newerror;
	} else {
	    alt_x = VAR(estimate_x) - d[0];
	    alt_y = VAR(estimate_y) - d[1];
	    VAR(estimate_x) += d[0];
	    VAR(estimate_y) += d[1];
	    olderror = newerror;
	}

	if (iterations > MAX_ITERATIONS) {
#ifdef FULLPC
	    printf
		("***********************************************************\n");
	    printf
		("APS algorithm could not converge after %d iterations\n",
		 MAX_ITERATIONS);
	    printf
		("***********************************************************\n");
#endif
	    return;
	}
    }

#ifndef FULLPC
    CLR_RED_LED_PIN();
    CLR_GREEN_LED_PIN();
    CLR_YELLOW_LED_PIN();
#endif

#ifdef FULLPC
    printf
	("***********************************************************\n");
    printf("Position estimate : (");
    fp_print(VAR(estimate_x));
    printf("\t");
    fp_print(VAR(estimate_y));
    printf(")\n");
    printf("Number of iterations: %d\n", iterations);
    printf
	("***********************************************************\n");
#endif
}


TOS_TASK(aps_algorithm)
{
    int             i = 0;

    VAR(aps_has_started) = 1;
    VAR(estimate_x) = 0;
    VAR(estimate_y) = 0;

    while (i < VAR(no_landmarks)) {
	VAR(estimate_x) += VAR(lms)[i].x;
	VAR(estimate_y) += VAR(lms)[i].y;

	i++;
    }

    VAR(estimate_x) =
	fp_div(VAR(estimate_x), fp_int_fix(VAR(no_landmarks)));
    VAR(estimate_y) =
	fp_div(VAR(estimate_y), fp_int_fix(VAR(no_landmarks)));

    aps_iterations();
}


void
check_if_landmark()
{
#ifdef FULLPC
    FILE           *fp = NULL;
    char            line[100],
                   *chr1 = NULL,
                   *chr2 = NULL;
    int             node,
                    x,
                    y;

    if ((fp = fopen(LANDMARK_FILE, "r")) == NULL) {
	printf("Couldn't open file %s\n", LANDMARK_FILE);
	exit(-1);
    }

    while (fgets(line, 100, fp)) {
	chr1 = strchr(line, '\t');
	if (chr1 == NULL) {
	    printf("Line %s has wrong format\n", line);
	    exit(-1);
	}
	*chr1 = '\0';

	node = atoi(line);

	chr2 = strchr(chr1 + 1, '\t');
	if (chr2 == NULL) {
	    printf("Line %s has wrong format\n", line);
	    exit(-1);
	}

	*chr2 = '\0';

	x = atoi(chr1 + 1);
	y = atoi(chr2 + 1);

	if (node == TOS_LOCAL_ADDRESS) {
	    printf("Node %d is a landmark: x = %d, y = %d\n", node, x, y);

	    VAR(lms)[0].x = fp_int_fix(x);
	    VAR(lms)[0].y = fp_int_fix(y);
	    VAR(lms)[0].dist = 0;

	    VAR(no_landmarks) = 1;
	    VAR(is_landmark) = 1;

	    break;
	}
    }

    if (!VAR(is_landmark)) {
	VAR(no_landmarks) = 0;
	VAR(is_landmark) = 0;
    }

    fclose(fp);
#else
    if (IS_LANDMARK == 1) {
	VAR(lms)[0].x = fp_int_fix(X_COORD);
	VAR(lms)[0].y = fp_int_fix(Y_COORD);
	VAR(lms)[0].dist = 0;

	VAR(no_landmarks) = 1;
	VAR(is_landmark) = 1;
    } else {
	VAR(no_landmarks) = 0;
	VAR(is_landmark) = 0;
    }
#endif
}


int
is_already_there(fixpoint x, fixpoint y)
{
    int             i = 0;

    while (i < VAR(no_landmarks)) {
	if (VAR(lms)[i].x == x && VAR(lms)[i].y == y)
	    return 1;

	i++;
    }

    return 0;
}


void
append_landmark(fixpoint x, fixpoint y, fixpoint dist)
{
    VAR(lms)[VAR(no_landmarks)].x = x;
    VAR(lms)[VAR(no_landmarks)].y = y;
    VAR(lms)[VAR(no_landmarks)].dist = dist;

    VAR(no_landmarks)++;
}


int
get_dist(int strength)
{
    return strength;
}


char            TOS_COMMAND(AD_HOC_POS_INIT) () {
    VAR(no_landmarks) = 0;
    VAR(is_landmark) = 0;
    VAR(aps_has_started) = 0;
    VAR(current_tick) = 0;

    check_if_landmark();

#ifndef FULLPC
    SET_RED_LED_PIN();
    SET_YELLOW_LED_PIN();
    SET_GREEN_LED_PIN();
#endif

    TOS_CALL_COMMAND(AD_HOC_POS_SUB_OUTPUT_INIT) ();

    // if (VAR(is_landmark)) 
    // TOS_CALL_COMMAND(AD_HOC_POS_OUTPUT)(VAR(head)->x, VAR(head)->y,
    // VAR(head)->dist);

    return 1;
}


char            TOS_COMMAND(AD_HOC_POS_START) () {
    return TOS_CALL_COMMAND(AD_HOC_POS_SUB_CLOCK_INIT) (tick1ps);
}


/*
 * this will check the table_dirty flag and send the table data 
 */
char
check_and_send_table()
{
    if (!VAR(table_dirty))
	return 1;

    return 1;
}


void            TOS_EVENT(AD_HOC_POS_CLOCK_EVENT) () {

    if (VAR(is_landmark)) {
	// see TICK_SP
	// we increment the current_tick and send the message only if it
	// is above TICK_SP
	VAR(current_tick)++;
	if (VAR(current_tick) > TICK_SP) {
	    VAR(current_tick) = 0;
	    printf("TICK_SP reached\n");
	} else {
	    return;
	}

	TOS_CALL_COMMAND(AD_HOC_POS_OUTPUT) (VAR(lms)[0].x, VAR(lms)[0].y,
					     VAR(lms)[0].dist);
    } else {
	/*
	 * we are not a landmark. see if the table_dirty flag is set, if
	 * so broadcast the entire table presently, the table_dirty flag
	 * will be set whenever the AD_HOC_POS_INPUT is fired. this is to
	 * ensure that the node periodically broadcasts the table to help
	 * new nodes to get the landmark positions. 
	 */
	return check_and_send_table();
    }

}


char            TOS_EVENT(AD_HOC_POS_OUTPUT_COMPLETE) (char success) {
    return 1;
}


char            TOS_EVENT(AD_HOC_POS_INPUT) (int fpx, int fpy, int dist,
					     int strength) {
    int             total_dist;
    fixpoint        fpdist;

#ifdef FULLPC
    printf("Message: ");
    fp_print(fpx);
    printf("\t");
    fp_print(fpy);
    printf("\t%d\n", strength);
#endif

    if (VAR(no_landmarks) < NUM_LANDMARKS_TO_USE) {
	total_dist = dist + get_dist(strength);

	fpdist = fp_int_fix(total_dist);

	if (!is_already_there(fpx, fpy))
	    append_landmark(fpx, fpy, fpdist);
    }

    if (!(VAR(is_landmark)) && (VAR(no_landmarks) == NUM_LANDMARKS_TO_USE)
	&& (!VAR(aps_has_started))) {
	TOS_POST_TASK(aps_algorithm);
    }

    /*
     * if we are not a landmark, then the table_dirty flag must be set 
     */
    VAR(table_dirty) = 1;

    return 1;
    // return TOS_CALL_COMMAND(AD_HOC_POS_OUTPUT)(x, y, total_dist);
}
