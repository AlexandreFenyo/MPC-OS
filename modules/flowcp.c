
/* ATTENTION : CORRIGER BUG MICP intercalle avec send/recv */

/* $Id: flowcp.c,v 1.3 2000/02/08 18:54:32 alex Exp $ */

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#ifdef _WITH_DDSCP

#include "../MPC_OS/mpcshare.h"

#include "driver.h"
#include "put.h"
#include "ddslrpp.h"
#include "data.h"
#include "flowcp.h"
#include "cmem.h"

#include <sys/systm.h>
#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <sys/syslog.h>

int RFCP_sap_id = -1;
int SFCP_sap_id = -1;
int MICP_sap_id = -1;

typedef enum SFCP_msg_type { SFCP_REQUEST, SFCP_ACK } SFCP_msg_type_t;
typedef enum RFCP_msg_type { RFCP_REQUEST, RFCP_ACK } RFCP_msg_type_t;
typedef enum MICP_msg_type { MICP_REQUEST, MICP_ACK } MICP_msg_type_t;

static node_t    SFCP_first_entry_node    = (node_t)    -1;
static channel_t SFCP_first_entry_channel = (channel_t) -1;

static node_t    RFCP_first_entry_node    = (node_t)    -1;
static channel_t RFCP_first_entry_channel = (channel_t) -1;

static node_t    SFCP_last_entry_node     = (node_t)    -1;
static channel_t SFCP_last_entry_channel  = (channel_t) -1;

static node_t    RFCP_last_entry_node     = (node_t)    -1;
static channel_t RFCP_last_entry_channel  = (channel_t) -1;

send_seq_entry_t send_seq_Y[MAX_NODES - 1][CHANNEL_RANGE];

recv_seq_entry_t recv_seq_X[MAX_NODES - 1][CHANNEL_RANGE];
recv_seq_entry_t recv_seq_Y[MAX_NODES - 1][CHANNEL_RANGE];

/************************************************************
 * RFCP                                                     *
 ************************************************************/

/*
 ************************************************************
 * RFCP_interrupt_received(minor, mi, data1, data2) :
 * interrupt handler for RFCP.
 * minor : board number.
 * mi    : the MI associated with this interrupt.
 * data1 : opaque data for RFCP.
 * data2 : opaque data for RFCP.
 ************************************************************
 */

static void
RFCP_interrupt_received(minor, mi, data1, data2)
                        int    minor;
                        mi_t   mi;
                        u_long data1;
                        u_long data2;
{
  int i;
  int size;
  node_t node;
  channel_t channel;
  seq_t seq;
  int request;
  lpe_entry_t entry;
  node_t prev_node, next_node;
  channel_t prev_channel, next_channel;

  TRY("RFCP_interrupt_received")

  request = SM_DATA_1(data1);
  seq     = SM_DATA_2(data1);
  node    = SM_DATA_1(data2);
  channel = SM_DATA_2(data2);

  switch(request) {
  case RFCP_REQUEST:
#ifdef DEBUG_HSL
    log(LOG_WARNING,
	"interrupt RFCP_REQUEST seq=%d current_seq=%d\n",
	seq,
	recv_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq);
#endif

    /* recv_seq_Y is only allowed to increase */
    if (SEQ_SUP_STRICT(recv_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq, seq)) {
      log(LOG_WARNING,
	  "RFCP_REQUEST inversion of packets (%d > %d)\n",
	  recv_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq,
	  seq);
      break;
    }

    /* We may free several entries at a time, on the sender */

    for (i = 0; i < PENDING_SEND_SIZE; i++)
      if (pending_send[i].valid == VALID &&
	  pending_send[i].dest == node &&
	  pending_send[i].channel == channel &&
	  SEQ_INF_STRICT(pending_send[i].seq, seq)) {
	send_phys_addr_entry_t *spaentry;

#ifdef DEBUG_HSL
	log(LOG_DEBUG,
	    "RFCP Request: cleaning pending_send[%d].seq = %d < %d\n",
	    i,
	    pending_send[i].seq,
	    seq);
#endif

	/* Free send_phys_addr[] entries */

	size = 0;
	spaentry = pending_send[i].pages;

	if (!spaentry) {
	  /* Should not happen... */
	  log(LOG_ERR,
	      "RFCP_REQUEST: spaentry null !\n");
	  return;
	}

	do {
	  spaentry->valid = INVALID;
	  size++;
	  send_phys_addr_alloc_entries--;
	} while ((spaentry = spaentry->next));

	/* Profiler */
	DECR_VAR_MULT(send_phys_addr, TRACE_DECR_SPA_ENTRY, size);

	/* Wake-up processes waiting for space in send_phys_addr[] */
	wakeup(&send_phys_addr_free_ident);

	/* callback */
	if (pending_send[i].fct) pending_send[i].fct(pending_send[i].param,
						     pending_send[i].size_sent);

	/* Free pending_send[] */
	pending_send[i].valid = INVALID;

	/* Profiler */
	DECR_VAR(pending_send_entry, TRACE_DECR_PS_ENTRY);

	/* wake-up processes */
	wakeup(&pending_send_free_ident);
      }

    /* Update recv_seq_Y[][] */
    recv_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq = seq;

    /* Reply with a Short Message */
    entry.page_length = 0;
    entry.control = NOR_MASK |
                    LMP_MASK |
                    LRM_ENABLE_MASK |
                    LMI_ENABLE_MASK |
                    SM_MASK |
                    put_get_mi_start(minor, RFCP_sap_id);
    entry.routing_part = node;
    entry.PRSA = (caddr_t) SM_PACK_DATA(RFCP_ACK, seq);
    entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);
    put_add_entry(minor,
		  &entry);
    break;

  case RFCP_ACK:
    /* If the distant value is not an image of the local value, break for the
       job to be done later */
    if (recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq != seq)
      break;

    /* Remove the job from the queue */

    if (recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_inside_list == TRUE) {
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_inside_list = FALSE;

      prev_node = recv_seq_X[NODE2NODEINDEX(node, my_node)]
                            [channel].RFCP_prev_node;
      prev_channel = recv_seq_X[NODE2NODEINDEX(node, my_node)]
                               [channel].RFCP_prev_channel;

      next_node = recv_seq_X[NODE2NODEINDEX(node, my_node)]
                            [channel].RFCP_next_node;
      next_channel = recv_seq_X[NODE2NODEINDEX(node, my_node)]
                               [channel].RFCP_next_channel;

      if (RFCP_first_entry_node == node && RFCP_first_entry_channel == channel) {
	/* We are at the beginning of the queue */
	RFCP_first_entry_node    = next_node;
	RFCP_first_entry_channel = next_channel;
      }

      if (RFCP_last_entry_node == node && RFCP_last_entry_channel == channel) {
	/* We are at the end of the queue */
	RFCP_last_entry_node    = prev_node;
	RFCP_last_entry_channel = prev_channel;
      }

      if (prev_node != (node_t) -1 && prev_channel != (channel_t) -1) {
	/* We are not at the beginning of the queue */
	recv_seq_X[prev_node][prev_channel].RFCP_next_node = next_node;
	recv_seq_X[prev_node][prev_channel].RFCP_next_channel = next_channel;
      }

      if (next_node != (node_t) -1 && next_channel != (channel_t) -1) {
	/* We are not at the end of the queue */
	recv_seq_X[next_node][next_channel].RFCP_prev_node = prev_node;
	recv_seq_X[next_node][next_channel].RFCP_prev_channel = prev_channel;
      }

      unregister_job();
    }
    break;

  default:
    /* Should not happen */
    log(LOG_ERR,
	"RFCP_interrupt_received error\n");
    break;
  }
}


/*
 ************************************************************
 * RFCP_do_jobs(minor) : flush the job queue.
 * minor : board number.
 *
 * return values : TRUE  : job queue not empty.
 *                 FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
RFCP_do_jobs(minor)
             int minor;
{
  lpe_entry_t entry;
  node_t node;
  channel_t channel;
  node_t new_node;

  TRY("RFCP_do_jobs")

  /* Return if no job to do */
  if (RFCP_first_entry_node == (node_t) -1 ||
      RFCP_first_entry_channel == (channel_t) -1)
    return FALSE;

  /* Prepare the Short Message */
  entry.page_length = 0;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, RFCP_sap_id);

  /* For each job, send a Short Message */
  for (node = RFCP_first_entry_node,
	 channel = RFCP_first_entry_channel;
       node != (node_t) -1 && channel != (channel_t) -1;
       new_node = recv_seq_X[NODE2NODEINDEX(node, my_node)]
	                    [channel].RFCP_next_node,
       channel = recv_seq_X[NODE2NODEINDEX(node, my_node)]
                           [channel].RFCP_next_channel,
       node = new_node) {
    entry.routing_part = node;
    entry.PRSA = (caddr_t)
      SM_PACK_DATA(RFCP_REQUEST,
		   recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq);
    entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);
    put_add_entry(minor,
		  &entry);
  }

  /* There are some remaining jobs => return TRUE */
  return TRUE;
}


/*
 ************************************************************
 * RFCP_start_process(minor, node, channel) : insert a job in
 * the job queue.
 * minor   : board number.
 * node    : node number for the job.
 * channel : channel number for the job.
 ************************************************************
 */

void
RFCP_start_process(minor, node, channel)
                   int       minor;
                   node_t    node;
                   channel_t channel;
{
  int s;
  lpe_entry_t entry;

  s = splhigh();

  /* Insert the channel at the end of the job queue if it isn't
     already in it */
  if (recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_inside_list == FALSE) {
    if (RFCP_first_entry_channel == (channel_t) -1 &&
	RFCP_first_entry_node == (node_t) -1) {
      /* The queue is empty */

      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_prev_node    =
	(node_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_prev_channel =
	(channel_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_next_node    =
	(node_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_next_channel =
	(channel_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].RFCP_inside_list  =
	TRUE;

      RFCP_first_entry_node    = RFCP_last_entry_node    = node;
      RFCP_first_entry_channel = RFCP_last_entry_channel = channel;

    } else {
      /* The queue is not empty */

      recv_seq_X[NODE2NODEINDEX(RFCP_last_entry_node, my_node)]
	        [RFCP_last_entry_channel].RFCP_next_node = node;
      recv_seq_X[NODE2NODEINDEX(RFCP_last_entry_node, my_node)]
                [RFCP_last_entry_channel].RFCP_next_channel = channel;

      recv_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].RFCP_next_node    = (node_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].RFCP_next_channel = (node_t) -1;
      recv_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].RFCP_prev_node    = RFCP_last_entry_node;
      recv_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].RFCP_prev_channel = RFCP_last_entry_channel;
      recv_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].RFCP_inside_list  = TRUE;

      RFCP_last_entry_node    = node;
      RFCP_last_entry_channel = channel;
    }
  }

  /* Register the job */
  register_job();

  /* Start the job */
  entry.page_length = 0;
  entry.routing_part = node;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, RFCP_sap_id);
  entry.PRSA = (caddr_t) SM_PACK_DATA(RFCP_REQUEST,
				      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq);
  entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);

  put_add_entry(minor,
		&entry);

  splx(s);
}


/*
 ************************************************************
 * RFCP_check_jobs() : check the job queue.
 *
 * return value : TRUE  : job queue not empty.
 *                FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
RFCP_check_jobs()
{
  TRY("RFCP_check_jobs")

  if (RFCP_first_entry_node == (node_t) -1 &&
      RFCP_first_entry_channel == (channel_t) -1)
    return FALSE;
  return TRUE;
}


/*
 ************************************************************
 * update_recv_seq_X(minor) : compute an entry in recv_seq_X[][].
 * node    : node for this entry.
 * channel : channel for this entry.
 ************************************************************
 */

void
update_recv_seq_X(node, channel)
                  node_t    node;
                  channel_t channel;
{
  int i;
  int min;

  TRY("update_recv_seq_X")

  min = MAX_SEQUENCE + 1;

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
    if ((*pending_receive)[i].valid == VALID &&
	(*pending_receive)[i].sender == node &&
	(*pending_receive)[i].channel == channel) {
      if (min == MAX_SEQUENCE + 1)
	min = (*pending_receive)[i].seq;
      else
	min = MIN_SEQ(min, (*pending_receive)[i].seq);
    }

  if (min == MAX_SEQUENCE + 1)
    /* There is no pending receive for this channel */
    recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq =
      recv_seq[NODE2NODEINDEX(node, my_node)][channel].seq;
  else
    /* There is some pending receive for this channel */
    recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq =
      min;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "update_recv_seq_X[%d][%d].seq = %d\n",
      NODE2NODEINDEX(node, my_node),
      channel,
      recv_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq);
#endif
}


/************************************************************
 * SFCP                                                     *
 ************************************************************/

/*
 ************************************************************
 * SFCP_interrupt_received(minor, mi, data1, data2) :
 * interrupt handler for SFCP.
 * minor : board number.
 * mi    : the MI associated with this interrupt.
 * data1 : opaque data for SFCP.
 * data2 : opaque data for SFCP.
 ************************************************************
 */

static void
SFCP_interrupt_received(minor, mi, data1, data2)
                        int    minor;
                        mi_t   mi;
                        u_long data1;
                        u_long data2;
{
  int i;
  node_t node;
  channel_t channel;
  seq_t seq;
  int request;
  lpe_entry_t entry;
  node_t prev_node, next_node;
  channel_t prev_channel, next_channel;

  TRY("SFCP_interrupt_received")

  request = SM_DATA_1(data1);
  seq     = SM_DATA_2(data1);
  node    = SM_DATA_1(data2);
  channel = SM_DATA_2(data2);

  switch(request) {
  case SFCP_REQUEST:
    /* send_seq_Y is only allowed to increase */
    if (SEQ_SUP_STRICT(send_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq, seq)) {
      log(LOG_WARNING,
	  "SFCP_REQUEST inversion of packets (%d > %d)\n",
	  send_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq,
	  seq);
      break;
    }

    /* Update send_seq_Y[][] */
    send_seq_Y[NODE2NODEINDEX(node, my_node)][channel].seq = seq;

    /* Reply with a Short Message */
    entry.page_length = 0;
    entry.control = NOR_MASK |
                    LMP_MASK |
                    LRM_ENABLE_MASK |
                    LMI_ENABLE_MASK |
                    SM_MASK |
                    put_get_mi_start(minor, SFCP_sap_id);
    entry.routing_part = node;
    entry.PRSA = (caddr_t) SM_PACK_DATA(SFCP_ACK, seq);
    entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);
    put_add_entry(minor,
		  &entry);

    /* Begin retransmissions */
    for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
      if ((*pending_receive)[i].valid == VALID &&
	  (*pending_receive)[i].sender == node &&
	  (*pending_receive)[i].channel == channel &&
	  SEQ_INF_STRICT((*pending_receive)[i].seq, seq) &&
	  (*pending_receive)[i].stage == STAGE_START)
	MICP_start_process(minor, (*pending_receive) + i);
    break;

  case SFCP_ACK:
    /* If the distant value is not an image of the local value, break for the
       job to be done later */
    if (send_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq != seq)
      break;

    /* Remove the job from the queue */

    if (send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_inside_list == TRUE) {
      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_inside_list = FALSE;

      prev_node = send_seq_X[NODE2NODEINDEX(node, my_node)]
	                    [channel].SFCP_prev_node;
      prev_channel = send_seq_X[NODE2NODEINDEX(node, my_node)]
                               [channel].SFCP_prev_channel;

      next_node = send_seq_X[NODE2NODEINDEX(node, my_node)]
                            [channel].SFCP_next_node;
      next_channel = send_seq_X[NODE2NODEINDEX(node, my_node)]
                               [channel].SFCP_next_channel;

      if (SFCP_first_entry_node == node && SFCP_first_entry_channel == channel) {
	/* We are at the beginning of the queue */
	SFCP_first_entry_node    = next_node;
	SFCP_first_entry_channel = next_channel;
      }

      if (SFCP_last_entry_node == node && SFCP_last_entry_channel == channel) {
	/* We are at the end of the queue */
	SFCP_last_entry_node    = prev_node;
	SFCP_last_entry_channel = prev_channel;
      }

      if (prev_node != (node_t) -1 && prev_channel != (channel_t) -1) {
	/* We are not at the beginning of the queue */
	send_seq_X[prev_node][prev_channel].SFCP_next_node = next_node;
	send_seq_X[prev_node][prev_channel].SFCP_next_channel = next_channel;
      }

      if (next_node != (node_t) -1 && next_channel != (channel_t) -1) {
	/* We are not at the end of the queue */
	send_seq_X[next_node][next_channel].SFCP_prev_node = prev_node;
	send_seq_X[next_node][next_channel].SFCP_prev_channel = prev_channel;
      }

      unregister_job();
    }
    break;

  default:
    /* Should not happen */
    log(LOG_ERR,
	"SFCP_interrupt_received error\n");
    break;
  }
}


/*
 ************************************************************
 * SFCP_do_jobs(minor) : flush the job queue.
 * minor : board number.
 *
 * return values : TRUE  : job queue not empty.
 *                 FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
SFCP_do_jobs(minor)
             int minor;
{
  lpe_entry_t entry;
  node_t node;
  channel_t channel;
  node_t new_node;

  TRY("SFCP_do_jobs")

  /* Return if no job to do */
  if (SFCP_first_entry_node == (node_t) -1 ||
      SFCP_first_entry_channel == (channel_t) -1)
    return FALSE;

  /* Prepare the Short Message */
  entry.page_length = 0;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, SFCP_sap_id);

  /* For each job, send a Short Message */
  for (node = SFCP_first_entry_node,
	 channel = SFCP_first_entry_channel;
       node != (node_t) -1 && channel != (channel_t) -1;
       new_node = send_seq_X[NODE2NODEINDEX(node, my_node)]
                            [channel].SFCP_next_node,
       channel = send_seq_X[NODE2NODEINDEX(node, my_node)]
                           [channel].SFCP_next_channel,
       node = new_node) {
    entry.routing_part = node;
    entry.PRSA = (caddr_t)
      SM_PACK_DATA(SFCP_REQUEST,
		   send_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq);
    entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);
    put_add_entry(minor,
		  &entry);
  }

  /* There are some remaining jobs => return TRUE */
  return TRUE;
}


/*
 ************************************************************
 * SFCP_start_process(minor, node, channel) : insert a job in
 * the job queue.
 * minor   : board number.
 * node    : node number for the job.
 * channel : channel number for the job.
 ************************************************************
 */

void
SFCP_start_process(minor, node, channel)
                   int       minor;
                   node_t    node;
                   channel_t channel;
{
  int s;
  lpe_entry_t entry;

  TRY("SFCP_start_process")

  s = splhigh();

  /* Insert the channel at the end of the job queue if it isn't
     already in it */
  if (send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_inside_list == FALSE) {

    if (SFCP_first_entry_channel == (channel_t) -1 &&
	SFCP_first_entry_node == (node_t) -1) {
      /* The queue is empty */

      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_prev_node    =
	(node_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_prev_channel =
	(channel_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_next_node    =
	(node_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_next_channel =
	(channel_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].SFCP_inside_list  =
	TRUE;

      SFCP_first_entry_node    = SFCP_last_entry_node    = node;
      SFCP_first_entry_channel = SFCP_last_entry_channel = channel;

    } else {
      /* The queue is not empty */

      send_seq_X[NODE2NODEINDEX(SFCP_last_entry_node, my_node)]
	        [SFCP_last_entry_channel].SFCP_next_node = node;
      send_seq_X[NODE2NODEINDEX(SFCP_last_entry_node, my_node)]
                [SFCP_last_entry_channel].SFCP_next_channel = channel;

      send_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].SFCP_next_node    = (node_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].SFCP_next_channel = (node_t) -1;
      send_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].SFCP_prev_node    = SFCP_last_entry_node;
      send_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].SFCP_prev_channel = SFCP_last_entry_channel;
      send_seq_X[NODE2NODEINDEX(node, my_node)]
                [channel].SFCP_inside_list  = TRUE;

      SFCP_last_entry_node    = node;
      SFCP_last_entry_channel = channel;
    }
  }

  /* Register the job */
  register_job();

  /* Start the job */
  entry.page_length = 0;
  entry.routing_part = node;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, SFCP_sap_id);
  entry.PRSA = (caddr_t) SM_PACK_DATA(SFCP_REQUEST,
				      send_seq_X[NODE2NODEINDEX(node, my_node)][channel].seq);
  entry.PLSA = (caddr_t) SM_PACK_DATA(my_node, channel);
  put_add_entry(minor,
		&entry);

  splx(s);
}


/*
 ************************************************************
 * SFCP_check_jobs() : check the job queue.
 *
 * return value : TRUE  : job queue not empty.
 *                FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
SFCP_check_jobs()
{
  TRY("SFCP_check_jobs")

  if (SFCP_first_entry_node == (node_t) -1 &&
      SFCP_first_entry_channel == (channel_t) -1)
    return FALSE;
  return TRUE;
}

/*
 ************************************************************
 * SFCP_check_channel(minor, dest, channel) : check the job queue
 * for a particular channel.
 * minor   : board number.
 * dest    : node number for the job.
 * channel : channel number for the job.
 *
 * MUST BE CALLED AT splhigh() PROCESSOR LEVEL.
 *
 * return value : TRUE  : job queue contains this channel.
 *                FALSE : job queue free of this channel.
 ************************************************************
 */

boolean_t
SFCP_check_channel(minor, dest, channel)
                   int minor;
		   node_t dest;
		   channel_t channel;
{
  node_t node;
  channel_t chan;
  node_t new_node;

  TRY("SFCP_check_channel")

  if (SFCP_first_entry_node != (node_t) -1 &&
      SFCP_first_entry_channel != (channel_t) -1)
    for (node = SFCP_first_entry_node,
	   chan = SFCP_first_entry_channel;
	 node != (node_t) -1 && chan != (channel_t) -1;
	 new_node = send_seq_X[NODE2NODEINDEX(node, my_node)]
                              [chan].SFCP_next_node,
	   chan = send_seq_X[NODE2NODEINDEX(node, my_node)]
                            [chan].SFCP_next_channel,
	   node = new_node)
      if (node == dest && chan == channel) return TRUE;
  return FALSE;
}


/************************************************************
 * MICP                                                     *
 ************************************************************/

/*
 ************************************************************
 * MICP_interrupt_received(minor, mi, data1, data2) :
 * interrupt handler for MICP.
 * minor : board number.
 * mi    : the MI associated with this interrupt.
 * data1 : opaque data for MICP.
 * data2 : opaque data for MICP.
 ************************************************************
 */

static void
MICP_interrupt_received(minor, mi, data1, data2)
                        int    minor;
                        mi_t   mi;
                        u_long data1;
                        u_long data2;
{
  int i;
  node_t node;
  int request;
  mi_t mi_to_clear;
  seq_t micp_seq;
  lpe_entry_t entry;

  TRY("MICP_interrupt_received")

  request     = SM_DATA_1_8_24(data1);
  mi_to_clear = SM_DATA_2_8_24(data1);
  node        = SM_DATA_1(data2);
  micp_seq    = SM_DATA_2(data2);

  /* Flush the LRM */
  put_flush_LRM(minor, mi_to_clear);

  switch(request) {
  case MICP_REQUEST:
    /* This invalidation makes entries in received_receive[][] to be valid
       only during periods when no data may be written inside them */
    recv_valid[NODE2NODEINDEX(MI_NODE_PART(mi_to_clear), my_node)]
      [MI_ID_PART(mi_to_clear)].general.valid = INVALID;

    /* Reply with an acknowledge */
    entry.page_length = 0;
    entry.routing_part = node;
    entry.control = NOR_MASK |
                    LMP_MASK |
                    LRM_ENABLE_MASK |
                    LMI_ENABLE_MASK |
                    SM_MASK |
                  put_get_mi_start(minor, MICP_sap_id);
    entry.PRSA = (caddr_t) SM_PACK_DATA_8_24(MICP_ACK,
					     mi_to_clear);
    entry.PLSA = (caddr_t) SM_PACK_DATA(my_node,
					micp_seq);
    put_add_entry(minor,
		  &entry);
    break;

  case MICP_ACK:
    for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
      if ((*pending_receive)[i].valid == VALID &&
	  (*pending_receive)[i].mi == mi_to_clear) {
	/* Don't treat the message if it is not a response to the last one
	   we have sent */
	if ((*pending_receive)[i].micp_seq != micp_seq)
	  continue;

	/* Reemission */

	/* stage is already set to STAGE_MICP
        (*pending_receive)[i].stage    = STAGE_MICP; */
	(*pending_receive)[i].micp_seq = 0;
	try_recv(*pending_receive + i);
        register_job();
      }

    break;

  default:
    /* Should not happen */
    log(LOG_ERR,
	"MICP_interrupt_received error\n");
    break;
  }
}


/*
 ************************************************************
 * MICP_do_jobs(minor) : flush the job queue.
 * minor : board number.
 *
 * return values : TRUE  : job queue not empty.
 *                 FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
MICP_do_jobs(minor)
             int minor;
{
  lpe_entry_t entry;
  int i;

  TRY("MICP_do_jobs")

  /* Prepare the Short Message */
  entry.page_length = 0;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, MICP_sap_id);

  /* Should be called at splhigh processor level */

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
    if ((*pending_receive)[i].valid == VALID &&
	(*pending_receive)[i].stage == STAGE_MICP) {
      /* Flush the LRM */
      put_flush_LRM(minor, (*pending_receive)[i].mi);

      /* Send the request */
      (*pending_receive)[i].micp_seq++;
      entry.routing_part = (*pending_receive)[i].sender;
      entry.PRSA = (caddr_t) SM_PACK_DATA_8_24(MICP_REQUEST,
					       (*pending_receive)[i].mi);
      entry.PLSA = (caddr_t) SM_PACK_DATA(my_node,
					  (*pending_receive)[i].micp_seq);
      put_add_entry(minor,
		    &entry);
    }  

  return TRUE;
}


/*
 ************************************************************
 * MICP_start_process(minor, prentry) : insert a job in
 * the job queue.
 * minor   : board number.
 * prentry : entry associated with the job.
 ************************************************************
 */

void
MICP_start_process(minor, prentry)
                   int                      minor;
		   pending_receive_entry_t *prentry;
{
  int s;
  lpe_entry_t entry;

  TRY("MICP_start_process")

  s = splhigh();

  /* Flush the LRM */
  put_flush_LRM(minor, prentry->mi);

  /* Remember there is a new job to do */
  prentry->stage    = STAGE_MICP;
  prentry->micp_seq = 0;

  /* Register the job */
  register_job();

  /* Start the job */
  entry.page_length = 0;
  entry.routing_part = prentry->sender;
  entry.control = NOR_MASK |
                  LMP_MASK |
                  LRM_ENABLE_MASK |
                  LMI_ENABLE_MASK |
                  SM_MASK |
                  put_get_mi_start(minor, MICP_sap_id);
  entry.PRSA = (caddr_t) SM_PACK_DATA_8_24(MICP_REQUEST,
					   prentry->mi);
  entry.PLSA = (caddr_t) SM_PACK_DATA(my_node,
				      prentry->micp_seq);
  put_add_entry(minor,
		&entry);

  splx(s);
}


/*
 ************************************************************
 * MICP_check_jobs() : check the job queue.
 *
 * return value : TRUE  : job queue not empty.
 *                FALSE : job queue empty.
 ************************************************************
 */

static boolean_t
MICP_check_jobs()
{
  int i;

  TRY("MICP_check_jobs")

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
    if ((*pending_receive)[i].valid == VALID &&
	(*pending_receive)[i].stage == STAGE_MICP)
      return TRUE;

  return FALSE;
}


/************************************************************
 * General Routines for FLOWCP                              *
 ************************************************************/

/*
 ************************************************************
 * register_job() : signal that a job queue may be not
 * empty.
 ************************************************************
 */

void
register_job(void)
{
  /* Must be called at splhigh processor level */

  TRY("register_job")

  if (timeout_in_progress == FALSE) {
    timeout(ddslrp_timeout,
	    (caddr_t) SLR_P_TIMEOUT_ARG,
	    SLR_P_TIMEOUT_TICKS);
    timeout_in_progress = TRUE;
  }
}


/*
 ************************************************************
 * unregister_job() : signal that a job queue may be empty.
 ************************************************************
 */

void
unregister_job(void)
{
  /* Must be called at splhigh processor level */

  TRY("unregister_job")

  if (timeout_in_progress == TRUE &&
      RFCP_check_jobs() == FALSE &&
      SFCP_check_jobs() == FALSE &&
      MICP_check_jobs() == FALSE &&
      SLRP_check_jobs() == FALSE) {
    untimeout(ddslrp_timeout,
	      (caddr_t) SLR_P_TIMEOUT_ARG
#ifdef _SMP_SUPPORT
	      , timeout_handle
#endif
	      );
    timeout_in_progress = FALSE;
  }
}


/*
 ************************************************************
 * flowcp_do_jobs(minor) : ask RFCP, SFCP, MICP and SLRP to flush
 * their job queues.
 * minor : board number.
 *
 * return values : TRUE  : their are some remaining jobs.
 *                 FALSE : all queues flushed.
 ************************************************************
 */

boolean_t
flowcp_do_jobs(minor)
               int minor;
{
  boolean_t ret1, ret2, ret3, ret4;

  TRY("flowcp_do_jobs")

  ret1 = RFCP_do_jobs(minor);
  ret2 = SFCP_do_jobs(minor);
  ret3 = MICP_do_jobs(minor);
  ret4 = SLRP_do_jobs(minor);

  return ret1 || ret2 || ret3 || ret4;
}


/*
 ************************************************************
 * init_flowcp(minor) : initialize the FLOWCP layers.
 * minor : board number.
 *
 * This function initialize the RFCP, SFCP and MICP layers.
 *
 * return values : 0       : success.
 *                 ENOBUFS : FLOWCP already initialized.
 *                 ...     : error code for put_attach_mi_range().
 ************************************************************
 */

int
init_flowcp(minor)
            int minor;
{
  int res;

  TRY("init_flowcp")

  SFCP_first_entry_node    = (node_t) -1;
  SFCP_first_entry_channel = (channel_t) -1;
  SFCP_last_entry_node     = (node_t) -1;
  SFCP_last_entry_channel  = (channel_t) -1;
  bzero(send_seq_Y, sizeof send_seq_Y);

  RFCP_first_entry_node    = (node_t) -1;
  RFCP_first_entry_channel = (channel_t) -1;
  RFCP_last_entry_node     = (node_t) -1;
  RFCP_last_entry_channel  = (channel_t) -1;
  bzero(recv_seq_X, sizeof recv_seq_X);
  bzero(recv_seq_Y, sizeof recv_seq_Y);

  if (RFCP_sap_id != -1 ||
      SFCP_sap_id != -1 ||
      MICP_sap_id != -1) {
    /* Should not happen */
    log(LOG_ERR,
	"init_flowcp error\n");
    return ENOBUFS;
  }

  RFCP_sap_id = put_register_SAP(minor,
				 NULL,
				 RFCP_interrupt_received);
  if (RFCP_sap_id < 0)
    return ENOBUFS;

  SFCP_sap_id = put_register_SAP(minor,
				 NULL,
				 SFCP_interrupt_received);
  if (SFCP_sap_id < 0) {
    put_unregister_SAP(minor,
		       RFCP_sap_id);
    RFCP_sap_id = -1;
    return ENOBUFS;
  }

  MICP_sap_id = put_register_SAP(minor,
				 NULL,
				 MICP_interrupt_received);
  if (MICP_sap_id < 0) {
    put_unregister_SAP(minor,
		       RFCP_sap_id);
    put_unregister_SAP(minor,
		       SFCP_sap_id);
    RFCP_sap_id = SFCP_sap_id = -1;
    return ENOBUFS;
  }

  res = put_attach_mi_range(minor,
			    RFCP_sap_id,
			    1);
  if (res < 0) {
    put_unregister_SAP(minor,
		       RFCP_sap_id);
    put_unregister_SAP(minor,
		       SFCP_sap_id);
    put_unregister_SAP(minor,
		       MICP_sap_id);
    RFCP_sap_id = SFCP_sap_id = MICP_sap_id = -1;
    return res;
  }

  res = put_attach_mi_range(minor,
			    SFCP_sap_id,
			    1);
  if (res < 0) {
    put_unregister_SAP(minor,
		       RFCP_sap_id);
    put_unregister_SAP(minor,
		       SFCP_sap_id);
    put_unregister_SAP(minor,
		       MICP_sap_id);
    RFCP_sap_id = SFCP_sap_id = MICP_sap_id = -1;
    return res;
  }

  res = put_attach_mi_range(minor,
			    MICP_sap_id,
			    1);
  if (res < 0) {
    put_unregister_SAP(minor,
		       RFCP_sap_id);
    put_unregister_SAP(minor,
		       SFCP_sap_id);
    put_unregister_SAP(minor,
		       MICP_sap_id);
    RFCP_sap_id = SFCP_sap_id = MICP_sap_id = -1;
    return res;
  }

  return 0;
}


/*
 ************************************************************
 * end_flowcp(minor) : close the FLOWCP layers.
 * minor : board number.
 *
 * This function closes the RFCP, SFCP and MICP layers.
 ************************************************************
 */

void
end_flowcp(minor)
           int minor;
{
  TRY("end_flowcp")

  if (RFCP_sap_id == -1 ||
      SFCP_sap_id == -1 ||
      MICP_sap_id == -1)
    /* Should not happen */
    log(LOG_ERR,
	"end_flowcp error\n");

  put_unregister_SAP(minor,
		     RFCP_sap_id);
  put_unregister_SAP(minor,
		     SFCP_sap_id);
  put_unregister_SAP(minor,
		     MICP_sap_id);

  RFCP_sap_id = SFCP_sap_id = MICP_sap_id = -1;
}

#endif

