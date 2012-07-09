// common place to put all shared structures
// used by APS

#ifndef FULLPC
#define scanf(x...) ;
#endif

#include "fixpoint.h"

#define Q_ERROR         (0x0)

#define Q_IS_LM         (0x1)
#define REP_IS_LM       (-Q_IS_LM)

#define Q_SET_LM        (0x2)
#define REP_SET_LM      (-Q_SET_LM)

#define Q_GET_COORD     (0x3)
#define REP_GET_COORD   (-Q_GET_COORD)

#define Q_GET_DV_ALGO   (0x4)
#define REP_GET_DV_ALGO (-Q_GET_DV_ALGO)

#define Q_SET_DV_ALGO   (0x5)
#define REP_SET_DV_ALGO (-Q_SET_DV_ALGO)

#define Q_DUMP_LMS      (0x6)
#define REP_DUMP_LMS    (-Q_DUMP_LMS)

#define Q_RERUN_APS          (0x8)
#define REP_RERUN_APS        (-Q_RERUN_APS)

#define Q_APS_STATUS         (0x9)
#define REP_APS_STATUS       (-Q_APS_STATUS)

#define Q_GET_CORRECTION     (0xa)
#define REP_GET_CORRECTION   (-Q_GET_CORRECTION)


/* this is the am msg type to use for query replies
 * this is *never* used by the motes.
 * only used by the node sending the queries (typically the ipaq)
 */
#define QUERY_HANDLER 6
#define REPLY_HANDLER 7

#define BYTECOPY(destPtr, src)  *(typeof(src) *)destPtr = (src)
#define INCR_VOID(voidPtr, srcVar) ((typeof(srcVar)*)voidPtr)++

typedef struct{
  int x;
  int y;
  char hops;
} coordinates_msg;

/* this defines the structure for a correction message */
typedef struct {
  int fpx;
  int fpy;
  int fpfactor;
} correction_msg;

/* this structure is for the query message */
typedef struct {
  // this is ignored when sending a query
  // but all replies will have the id of the replier
  char node_id;
  char toq;     // query type
  char data[30 - 2];   // size is DATA_LENGTH - 2
} query_msg;

typedef struct coords{
  fixpoint x;
  fixpoint y;
  char hops;
}landmarks;

