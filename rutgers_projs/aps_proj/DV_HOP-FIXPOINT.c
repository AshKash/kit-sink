/*
 * based on AD_HOC_POS this implements the DV hop algorithm. 
 */
#ifdef FULLPC
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#endif

#include "DV_HOP.h"
#include "aps_msgs.h"
#include "tos.h"

#ifdef FULLPC
#define LANDMARK_FILE "landmarks.txt"
#else
#define IS_LANDMARK 0		/* If 1 then it is a landmark, otherwise
				 * not a landmark */
#define X_COORD 2
#define Y_COORD 2
#endif

#define EPSILON 0x3333		/* Epsilon = 0.2 */
#define MAX_ITERATIONS 255	/* Exit from loop if algorithm has not */
					 /*
					  * converged in MAX_ITERATIONS 
					  */


// the clock doesnt seem to differentiate from tick1ps and tick1000ps!
// this tries to fix it
#define TICK_SP 10		// means tick once in x seconds (or clock
				// ticks)

/****************** !!!!IMPORTANT!!!! UNDEF THIS FOR REAL USE ********************/
/*
 * this will only enable the query system, dv_hop and aps triangualtaion
 * is disabled 
 */

/*
 * every node stores the (x,y,hop) triplets of all known landmarks in the
 * network. 
 */
// WARNING!!! touch these with real care
// do not modify the variable type
// you may have to see APS_QUERY.c, changes made here will break it
#define TOS_FRAME_TYPE DV_HOP_frame
TOS_FRAME_BEGIN(DV_HOP_frame)
{
    unsigned char   no_landmarks;
    unsigned char   is_landmark;
    fixpoint        estimate_x;
    fixpoint        estimate_y;
    fixpoint        correction_factor;
    unsigned char   aps_iterations;
    landmarks       lms[NUM_LANDMARKS_TO_USE];
    unsigned char   current_tick;	// see TICK_SP
}

TOS_FRAME_END(DV_HOP_frame);

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

	tmp_length = fp_sqrt(fp_multiply(tmp_x, tmp_x) +
			     fp_multiply(tmp_y, tmp_y));
	a[i][0] = -(fp_div(tmp_x, tmp_length));
	a[i][1] = -(fp_div(tmp_y, tmp_length));
	b[i] = tmp_length -
	    fp_multiply(fp_int_fix((int) VAR(lms)[i].hops),
			VAR(correction_factor));

	i++;
    }

}

void
aps_iterations()
{

    /*
     * the aps algo using DV it is very important that the corrections be
     * applied prior to calculating the cordinates 
     */

    fixpoint        b[NUM_LANDMARKS_TO_USE],
                    d[NUM_COORDINATES],
                    a[NUM_LANDMARKS_TO_USE][NUM_COORDINATES];
    fixpoint        newerror = 0,
                    olderror = fp_int_fix(500);
    fixpoint        alt_x = 0,
                    alt_y = 0;

    while (1) {
	VAR(aps_iterations)++;

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

	if (VAR(aps_iterations) > MAX_ITERATIONS) {
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
    printf("Number of iterations: %d\n", VAR(aps_iterations));
    printf
	("***********************************************************\n");
#endif

}

TOS_TASK(compute_corrections)
{

    fixpoint        tmpx,
                    tmpy,
                    total_dist = 0;
    unsigned char   total_hops = 0;
    unsigned char   i;

    (VAR(correction_factor) = 1);
    // calculate the average hop distance
    for (i = 1; i < VAR(no_landmarks); i++) {
	tmpx = VAR(lms)[0].x - VAR(lms)[i].x;
	tmpy = VAR(lms)[0].y - VAR(lms)[i].y;

	total_dist +=
	    fp_sqrt(fp_multiply(tmpx, tmpx) + fp_multiply(tmpy, tmpy));
	total_hops += VAR(lms)[i].hops;
    }
    VAR(correction_factor) =
	fp_div(total_dist, fp_int_fix((int) total_hops));

}

TOS_TASK(aps_algorithm)
{

    int             i = 0;

    VAR(aps_iterations) = 1;
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
    int             node,
                    x,
                    y;

    if ((fp = fopen(LANDMARK_FILE, "r")) == NULL) {
	printf("Couldn't open file %s\n", LANDMARK_FILE);
	exit(-1);
    }

    while (fscanf(fp, "%d %d %d", &node, &x, &y) != EOF) {
	printf("Got: %d, %d, %d\n", node, x, y);
	if (node == TOS_LOCAL_ADDRESS) {
	    printf("Node %d is a landmark: x = %d, y = %d\n", node, x, y);

	    VAR(lms)[0].x = fp_int_fix(x);
	    VAR(lms)[0].y = fp_int_fix(y);
	    VAR(lms)[0].hops = 0;

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
	VAR(lms)[0].hops = 0;

	VAR(no_landmarks) = 1;
	VAR(is_landmark) = 1;
    } else {
	VAR(no_landmarks) = 0;
	VAR(is_landmark) = 0;
    }
#endif
}

/*
 * scans the table to find fpx and fpy pairs. if present, returns the
 * index. otherwise, returns a -1 
 */
int
is_already_there(fixpoint x, fixpoint y)
{
    int             i = 0;

    do {
	if (VAR(lms)[i].x == x && VAR(lms)[i].y == y
	    && VAR(lms)[i].hops != -1)
	    return i;
    } while (i++ < VAR(no_landmarks));

    return -1;
}


void
append_landmark(fixpoint x, fixpoint y, char hops)
{
    // prevent array out of ranges
    if (VAR(no_landmarks) >= NUM_LANDMARKS_TO_USE)
	VAR(no_landmarks) = NUM_LANDMARKS_TO_USE - 1;
    VAR(lms)[VAR(no_landmarks)].x = x;
    VAR(lms)[VAR(no_landmarks)].y = y;
    VAR(lms)[VAR(no_landmarks)].hops = hops;

    VAR(no_landmarks)++;
}


char            TOS_COMMAND(DV_HOP_INIT) () {
    int             i = 0;
    VAR(no_landmarks) = 0;
    VAR(is_landmark) = 0;
    VAR(aps_iterations) = 0;
    VAR(current_tick) = 0;
    VAR(correction_factor) = 0;

    for (i = 0; i < NUM_LANDMARKS_TO_USE; i++)
	VAR(lms)[i].hops = -1;
    check_if_landmark();

#ifndef FULLPC
    SET_RED_LED_PIN();
    SET_YELLOW_LED_PIN();
    SET_GREEN_LED_PIN();
#endif

    TOS_CALL_COMMAND(DV_HOP_SUB_OUTPUT_INIT) ();

    return 1;
}


char            TOS_COMMAND(DV_HOP_START) () {
    return TOS_CALL_COMMAND(DV_HOP_SUB_CLOCK_INIT) (tick1ps);
}

void            TOS_EVENT(DV_HOP_CLOCK_EVENT) () {


    // see TICK_SP
    // we increment the current_tick and send the message only if
    // it is above TICK_SP
    VAR(current_tick)++;
    if (VAR(current_tick) > TICK_SP) {
	VAR(current_tick) = 0;
    }
    // make sure you dont call more than one COMMAND in on clock tick!
    if (VAR(current_tick) == 0 && VAR(is_landmark)) {
	/*
	 * send the coordinates 
	 */
	printf("TICK_SP: ");
	fp_print(VAR(lms)[0].x);
	printf(", ");
	fp_print(VAR(lms)[0].y);
	printf(", %d\n", VAR(lms)[0].hops);

	TOS_CALL_COMMAND(DV_HOP_OUTPUT) (VAR(lms)[0].x, VAR(lms)[0].y,
					 VAR(lms)[0].hops);
    }
    /*
     * send the correction 
     */
    if (VAR(current_tick) == 1 && VAR(correction_factor)) {
	printf("SEND_CORRECTION: (");
	fp_print(VAR(lms)[0].x);
	printf(", ");
	fp_print(VAR(lms)[0].y);
	printf(") -> ");
	fp_print(VAR(correction_factor));
	printf("\n");
	TOS_CALL_COMMAND(DV_HOP_CORRECTION_OUTPUT) (VAR(lms)[0].x,
						    VAR(lms)[0].y,
						    VAR
						    (correction_factor));

    }
}


char            TOS_EVENT(DV_HOP_OUTPUT_COMPLETE) (char success) {
    return 1;
}

/*
 * called when the query comes 
 */
char            TOS_EVENT(DV_HOP_QUERY_INPUT) (void *msg) {
    query_msg      *q_msg = (query_msg *) msg;

    q_msg->node_id = TOS_LOCAL_ADDRESS;
    switch (q_msg->toq) {
    case Q_IS_LM:
	printf("IS_LM: %d\n", VAR(is_landmark));
	if (VAR(is_landmark)) {
	    q_msg->data[0] = 1;
	} else {
	    q_msg->data[0] = 0;
	}
	break;

    case Q_SET_LM:{
	    char            set_lm = q_msg->data[0];
	    landmarks      *lmptr = (landmarks *) & q_msg->data[1];

	    printf("SET_LM; %d (", set_lm);
	    fp_print(lmptr->x);
	    printf(", ");
	    fp_print(lmptr->y);
	    printf("\n");

	    if (set_lm) {
		VAR(is_landmark) = 1;
		// if this is a landmark, then lms[0] must be its coords
		VAR(lms)[VAR(no_landmarks)++] = VAR(lms)[0];
		VAR(lms)[0] = *lmptr;
	    } else {
		VAR(is_landmark) = 0;
		VAR(lms)[0] = VAR(lms)[VAR(no_landmarks)--];
	    }
	    break;
	}
    case Q_GET_COORD:{
	    landmarks      *lmptr = (landmarks *) & q_msg->data[0];

	    if (VAR(is_landmark)) {
		lmptr->x = VAR(lms)[0].x;
		lmptr->y = VAR(lms)[0].y;
	    } else {
		lmptr->x = VAR(estimate_x);
		lmptr->y = VAR(estimate_y);
	    }

	    printf("GET_COORD: (");
	    fp_print(lmptr->x);
	    printf(", ");
	    fp_print(lmptr->y);
	    printf("\n");

	    break;

    case Q_GET_DV_ALGO:{
		// unimplemented
		q_msg->toq = Q_ERROR;
		q_msg->data[0] = Q_GET_DV_ALGO;
		strcpy(q_msg->data + 1, "Unimplemented");
	    }
	    break;

    case Q_SET_DV_ALGO:
	    // unimplemented
	    q_msg->toq = Q_ERROR;
	    q_msg->data[0] = Q_SET_DV_ALGO;
	    strcpy(q_msg->data + 1, "Unimplemented");
	}
	break;

    case Q_DUMP_LMS:{
	    unsigned char   i;
	    unsigned char   whence = q_msg->data[0];	// index of table
	    // to start from
	    landmarks      *lmsptr;
	    typeof(VAR(no_landmarks)) * num_lms = &q_msg->data[1];

	    *num_lms = 0;
	    printf("DUMP_LMS: [%d, %d, %d]\t", VAR(no_landmarks),
		   sizeof(*num_lms), sizeof(VAR(no_landmarks)));
	    lmsptr = num_lms + 1;

	    for (i = whence; i < VAR(no_landmarks) &&
		 i < DATA_LENGTH / sizeof(landmarks) + whence; i++) {
		*lmsptr = VAR(lms)[i];
		(*num_lms)++;

		printf("(");
		fp_print(lmsptr->x);
		printf(", ");
		fp_print(lmsptr->y);
		printf(", %d)\t", lmsptr->hops);
		lmsptr++;

	    }
	    printf("\n");
	    break;
	}
    case Q_GET_CORRECTION:{
	    printf("GET_CORRECTION: ");
	    fp_print(VAR(correction_factor));
	    printf("\n");

	    BYTECOPY(q_msg->data, VAR(correction_factor));
	}
	break;

    case Q_RERUN_APS:
	printf("RERUN_APS: ");
	fp_print(VAR(aps_iterations));
	printf("\n");

	VAR(aps_iterations) = 0;
	break;

    case Q_APS_STATUS:
	printf("APS_STATUS: ");
	fp_print(VAR(aps_iterations));
	printf("\n");

	q_msg->data[0] = VAR(aps_iterations);
	break;

    default:
	q_msg->toq = Q_ERROR;
	strcpy(q_msg->data, "Invalid TOQ");
	break;
    }
    if (q_msg->toq != Q_ERROR)
	q_msg->toq = -q_msg->toq;
    return TOS_CALL_COMMAND(DV_HOP_QUERY_REPLY) ((void *) q_msg);
}

/*
 * fired when a correction comes in 
 */
char            TOS_EVENT(DV_HOP_CORRECTION_INPUT) (int fpx, int fpy,
						    int fpfactor) {

    // (fpx,fpy) has sent the correction factor fpfactor
    // right now, we simply dont care who has sent the correction
    // we store only the first correction factor, since it has higher
    // chances
    // of being from the local landmark, thereby being more accurate
    if (!VAR(correction_factor)) {
	printf("GOT CORRECTION: (%d) ", !!VAR(correction_factor));
	fp_print(fpx);
	printf(", ");
	fp_print(fpy);
	printf(", ");
	fp_print(fpfactor);
	printf("\n");
	VAR(correction_factor) = fpfactor;
    }
    // do not propagate the correction_factor here, it will cause looping
    // see the clock fire event, corrections are sent periodically
    // landmarks calculate them and broadcast
    // non landmarks, store the first received correction and periodically 
    // 
    // broadcast


    return 1;

}

/*
 * this gets fired when some node broadcasts the quadruple 
 */
char            TOS_EVENT(DV_HOP_INPUT) (int fpx, int fpy, char hops) {

    int             index;
    char            total_hops;

    /*
     * algo: if the landmark doesnt exist and NUM_LANDMARKS_TO_USE is
     * still not reached, then add the landmark to the table. if the
     * landmark does exist, then we add the landmark to the table only if
     * hops is smaller (or equal) than the existing one. and also relay
     * the information. 
     */
    index = is_already_there(fpx, fpy);
    total_hops = hops + 1;

    /*
     * debug stuff 
     */
    printf("Message: (index %d) ", index);
    fp_print(fpx);
    printf("\t");
    fp_print(fpy);
    printf("\t%d\n", hops);

    if (index == -1 && VAR(no_landmarks) < NUM_LANDMARKS_TO_USE)
	append_landmark(fpx, fpy, total_hops);
    if (index != -1) {
	if (total_hops > 0 && ((total_hops) <= VAR(lms)[index].hops)) {
	    VAR(lms)[index].hops = total_hops;
	    printf("Relay: %d, %d, %d\n", fp_integer(fpx), fp_integer(fpy),
		   total_hops);
	    TOS_CALL_COMMAND(DV_HOP_OUTPUT) (fpx, fpy, total_hops);
	}
    }
    printf
	("no_landmarks: %d, NUM_LM: %d, is_lm: %d, aps_iter: %d, cf: %d\n",
	 VAR(no_landmarks), NUM_LANDMARKS_TO_USE, VAR(is_landmark),
	 VAR(aps_iterations), VAR(correction_factor));
    /*
     * suppose the NUM_LANDMARKS_TO_USE is reached, if we are a landmark,
     * then we need to calculate the correction factor and propogate it.
     * if we are not a landmark, then do the triangulation. 
     */
    if (VAR(no_landmarks) == NUM_LANDMARKS_TO_USE) {
	int             i;
	printf("hello!!!!\n");
	if (!VAR(is_landmark) && !VAR(aps_iterations)
	    && VAR(correction_factor)) {
	    // not a landmark and triangulation needs to be done
	    // we also need to see if the correction has been received, if 
	    // 
	    // so we can
	    // calculate the posistion. else we will have to wait for it
	    printf("########################\n");
	    for (i = 0; i < VAR(no_landmarks); i++) {
		printf("%d: (%d, %d) at %d hops\n", i,
		       fp_integer(VAR(lms)[i].x),
		       fp_integer(VAR(lms)[i].y), VAR(lms)[i].hops);
	    }
	    printf("########################\n");

	    TOS_POST_TASK(aps_algorithm);

	} else if (VAR(is_landmark)) {
	    // landmark, need to propogate the corrections
	    if (!VAR(correction_factor))
		TOS_POST_TASK(compute_corrections);

	}
    }

    return 1;
}
