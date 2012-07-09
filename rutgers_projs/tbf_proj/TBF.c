/*
  $Log$
*/

#include "tos.h"
#include "TBF.h"
#include "tbf_hdr.h"
#define TBF_MAIN
#include "mapping.h"

#define TOS_FRAME_TYPE TBF_frame
TOS_FRAME_BEGIN(TBF_frame)
{
  struct tbf_hdr hdr;
}
TOS_FRAME_END(TBF_frame);

char TOS_COMMAND(TBF_INIT)(void)
{
  TOS_CALL_COMMAND(TBF_SUB_OUTPUT_INIT)();
  TOS_CALL_COMMAND(TBF_SUB_CLOCK_INIT)(tick1ps);
  return (1);
}

char TOS_COMMAND(TBF_START)(void)
{
  return (1);
}

void TOS_EVENT(TBF_CLOCK_FIRE)()
{
}

char TOS_EVENT(TBF_OUTPUT_COMPLETE)(char success)
{
  return (1);
}

char TOS_EVENT(TBF_INPUT) (void)
{
  return (1);
}

char TOS_EVENT(TBF_SUB_MSG_SEND_DONE) (TOS_MsgPtr sbuffer)
{
  return (1);
}

TOS_MsgPtr TOS_MSG_EVENT(TBF_BEACON) (TOS_MsgPtr msg)
{
  return (msg);
}

TOS_MsgPtr TOS_MSG_EVENT(TBF_RECEIVE) (TOS_MsgPtr msg)
{
  moteid_t *id = &msg->addr;
  struct tbf_hdr *ptbf = (struct tbf_hdr *)msg->data;
  int (*func)(struct tbf_hdr *ptbf, moteid_t *id);

  // call the requested policy for this pkt
  if (ptbf->policy == POLICY_MAX_PROGRESS) func = policy_max_progress;
  else if (ptbf->policy == POLICY_MIN_DISTANCE) func = policy_min_distance;

  else return msg;

  if (func(ptbf, moteid) == PKT_FWD) {
    // FIXME: code to fwd the pkt
  }
  else return msg;

  // notify the higher layer if the process flag is set
  if (ptbf->proc == 1)
  if (tbf_tos_list[ptbf->tos] != NULL)
  return (msg);
}
