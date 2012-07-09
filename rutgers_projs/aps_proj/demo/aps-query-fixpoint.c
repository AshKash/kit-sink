#include "../system/include/aps_msgs.h"
#include "../system/include/fixpoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "mote-gw.h"

/*
 * to avoid 2 threads writing to output simultaneously 
 */
pthread_mutex_t output = PTHREAD_MUTEX_INITIALIZER;
#define OUTPUT_LOCK pthread_mutex_lock(&output);
#define OUTPUT_UNLOCK pthread_mutex_unlock(&output);

extern int      errno;

/*
 * this func is called by a seperate thread, which is started on init 
 */
/*
 * prints the reply packet go to stdout, format: ***REPLY*** <node_id>
 * <toq> <data> ***REPLY*** 
 */
TOS_Msg        *
got_reply(TOS_Msg * am_pkt)
{
    query_msg      *qmptr = (query_msg *) am_pkt->data;

    OUTPUT_LOCK;
    printf("***REPLY*** %d ", qmptr->node_id);

    switch (qmptr->toq) {
    case Q_ERROR:
	printf("Q_ERROR '%s'", qmptr->data);
	break;
    case REP_IS_LM:
	printf("REP_IS_LM %d", (int) *qmptr->data);
	break;

    case REP_SET_LM:{
	    fixpoint       *fp = (fixpoint *) (qmptr->data + 1);
	    printf("REP_SET_LM %d ", (int) *qmptr->data);
	    fp_print(*fp++);
	    printf(" ");
	    fp_print(*fp++);
	}
	break;

    case REP_GET_COORD:{
	    fixpoint       *fp = (fixpoint *) qmptr->data;
	    printf("REP_GET_COORD ");
	    fp_print(*fp++);
	    printf(" ");
	    fp_print(*fp++);
	}
	break;

    case REP_DUMP_LMS:{
	    unsigned char   whence = qmptr->data[0];
	    unsigned char   no_landmarks = qmptr->data[1];
	    landmarks      *lmsptr = (landmarks *) & qmptr->data[2];
	    int             i;

	    printf("REP_DUMP_LMS %d %d", whence, no_landmarks);

	    for (i = 0; i < no_landmarks && i < AM_PKT_LEN; i++) {

		printf("(");
		fp_print(lmsptr->x);
		printf(", ");
		fp_print(lmsptr->y);
		printf(", %d)\t", lmsptr->hops);
		lmsptr++;
	    }
	}

	break;

    case REP_RERUN_APS:
	printf("REP_RERUN_APS");
	break;

    case REP_APS_STATUS:
	printf("REP_APS_STATUS %d", qmptr->data[0]);
	break;

    case REP_GET_CORRECTION:
	printf("REP_GET_CORRECTION ");
	fp_print(*(fixpoint *) qmptr->data);
	break;

    default:
	printf("UNKNOWN");
    }

    printf(" ***REPLY***\n");
    fflush(stdout);
    OUTPUT_UNLOCK;

    /*
     * Packet must now be dropped 
     */
    return NULL;
}

/*
 * reads commands from the CL and writes the appropriate query to TOS_Msg 
 */
/*
 * format: QUERY <nodeid> <toq> <toq args> 
 */
/*
 * This does OUTPUT_LOCK, caller must do OUTPUT_UNLOCK 
 */
int
read_cl(TOS_Msg * am)
{
    char            buf[1024];
    char            toq[1024],
                    word1[256];
    int             nodeid;
    query_msg      *qmptr = (query_msg *) am->data;
    int             n = 0,
                    items;

    /*
     * read the CL args into the buffer 
     */
    fflush(stdout);
    fprintf(stderr, "> ");
    fflush(stdout);
    memset(buf, 0, sizeof(buf));
    if (fgets(buf, sizeof(buf), stdin) == NULL)
	return 2;

    OUTPUT_LOCK;		// make sure this is after fgets otherwise 
				// 
    // blocks

    /*
     * process the line and fill am pkt get next few words 
     */
    items = sscanf(buf, "%s %d %s%n", word1, &nodeid, toq, &n);

    if (items != 3 || strncasecmp(word1, "QUERY", strlen("QUERY")) != 0) {
	fprintf(stderr, "Error '%s'", buf);
	return 0;
    }

    qmptr->node_id = nodeid;
    am->addr = nodeid;
    am->type = QUERY_HANDLER /* QUERY_HANDLER */ ;
    am->group = LOCAL_GROUP;

    fprintf(stderr, "%s, %d, %s\n", word1, nodeid, toq);

    if (!strcasecmp("IS_LM", toq)) {
	fprintf(stderr, "Error is_lm\n");
	qmptr->toq = Q_IS_LM;
	return 1;

    } else if (!strcasecmp("SET_LM", toq)) {
	fixpoint        fpx,
	                fpy;
	double          x,
	                y;
	int             setlm;

	fprintf(stderr, "set_lm: ");
	items = sscanf(buf + n, "%d %lf %lf", &setlm, &x, &y);
	if (items != 3) {
	    fprintf(stderr, "Error set_lm %d\n", items);
	    return 0;
	}

	fprintf(stderr, "%d, %lf, %lf\n", setlm, x, y);
	fpx = fp_fix(x);
	fpy = fp_fix(y);

	qmptr->toq = Q_SET_LM;
	qmptr->data[0] = (char) setlm;
	((fixpoint *) (qmptr->data + 1))[0] = fpx;
	((fixpoint *) (qmptr->data + 1))[1] = fpy;

	return 1;

    } else if (!strcasecmp("GET_COORD", toq)) {
	fprintf(stderr, "Error get_coord\n");
	qmptr->toq = Q_GET_COORD;
	return 1;

    } else if (!strcasecmp("DUMP_LMS", toq)) {
	int             whence;

	fprintf(stderr, "dump_lms\n");
	items = sscanf(buf + n, "%d", &whence);
	if (items != 1) {
	    fprintf(stderr, "Error dump_lms %d\n", items);
	    return 0;
	}

	qmptr->toq = Q_DUMP_LMS;
	qmptr->data[0] = (unsigned char) whence;
	return 1;

    } else if (!strcasecmp("RERUN_APS", toq)) {
	fprintf(stderr, "Error rerun_aps\n");
	qmptr->toq = Q_RERUN_APS;
	return 1;

    } else if (!strcasecmp("APS_STATUS", toq)) {
	fprintf(stderr, "Error aps_status\n");
	qmptr->toq = Q_APS_STATUS;
	return 1;

    } else if (!strcasecmp("GET_CORRECTION", toq)) {
	fprintf(stderr, "Error get_correction\n");
	qmptr->toq = Q_GET_CORRECTION;
	return 1;

    } else {
	/*
	 * error 
	 */
	fprintf(stderr, "Error! bad query: %s\n", toq);
	return 0;
    }

    /*
     * send back status of conversion 
     */
    return 1;
}


int
main(int argc, char **argv)
{

    int             server_socket = client_init(argc, argv);
    TOS_Msg         am;
    int             st;

    if (server_socket < 0) {
	fprintf(stderr, "Failed to connect to server\n");
	exit(1);
    }


    /*
     * Register the am handler 
     */
    reg_am_type(REPLY_HANDLER, got_reply);

    /*
     * form the packet by reading CL 
     */
    while (1) {
	memset(&am, 0, sizeof(am));
	st = read_cl(&am);
	OUTPUT_UNLOCK;		// important! see read_cl()

	if (st == 1) {
	    /*
	     * send the packet 
	     */
	    send_tos_msg(server_socket, &am);
	    OUTPUT_LOCK;
	    fprintf(stderr, "send pkt %d\n", am.data[0]);
	    OUTPUT_UNLOCK;

	} else if (st == 2) {
	    /*
	     * exit 
	     */
	    break;

	} else {
	    /*
	     * error 
	     */
	    continue;

	}
    }
    /*
     * Exit 
     */
    fflush(stdout);
    fflush(stderr);
    exit(1);
}
