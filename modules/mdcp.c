
/* $Id: mdcp.c,v 1.4 2000/02/08 18:54:32 alex Exp $ */

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "put.h"
#include "ddslrpp.h"
#include "ddslrpv.h"
#include "data.h"
#include "cmem.h"
#include "mdcp.h"

#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/syslog.h>

#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <vm/lock.h>
#endif

/* Headers for the select operations. */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/filedesc.h>
#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/signalvar.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/malloc.h>
#include <vm/vm.h>

#ifdef _SMP_SUPPORT
#include <sys/poll.h>
#endif

/* struct selinfo and associated functions. */
#include <sys/select.h>

/* MDCP stands for Multi-Deposit Channelized Protocol. */

#define MDCP_SEND_BUF_SIZE  1024
#define MDCP_SEND_NBUFS     4 /* Multiple buffers for concurrency on a same
			         channel. */

#define MDCP_RECV_BUF_SIZE  16384
#define MDCP_RECV_NBUFS     4 /* Multiple buffers for multiple receives
				 before any send (and for concurrency). */

#if MDCP_RECV_BUF_SIZE < MDCP_SEND_BUF_SIZE
#error Invalid buffer size
#endif

typedef struct _mdcp_send_head {
  boolean_t in_use;
  /* This variable is a wired space sent in a piggy backed SEND operation
     if input buffering. */
  ssize_t   size_sent;
} mdcp_send_head_t;

typedef struct _mdcp_send_buffer {
  mdcp_send_head_t head;
  char data[MDCP_SEND_BUF_SIZE];
} mdcp_send_buffer_t;

typedef mdcp_send_buffer_t send_buffers_t[MDCP_SEND_NBUFS];

typedef struct _mdcp_recv_head {
  /*
   * TRUE indicates a pending receive on this buffer.
   * FALSE indicates availability of data in this buffer.
   * If FALSE, size contains the size of valid data in the buffer.
   */
  boolean_t in_use;
  size_t    size;
} mdcp_recv_head_t;

typedef struct _mdcp_recv_buffer {
  mdcp_recv_head_t head; /* also a waiting condition */
  char data[MDCP_RECV_BUF_SIZE];
} mdcp_recv_buffer_t;

typedef mdcp_recv_buffer_t recv_buffers_t[MDCP_RECV_NBUFS];

int send_buffer_freed = 0; /* a waiting condition : see send_buffers */

typedef struct _mdcp_slots_info {
  send_buffers_t *send_buffers; /* also a waiting condition */
  /* In fact, send_buffers is not a waiting condition : there is a general
     waiting condition for all channels, that aggregates those
     conditions : when a send buffer is freed, a wake-up on send_buffer_freed
     is performed. The disadvantage is that every process waiting
     for a send buffer is wake up, instead of the only ones using
     this particular channel. But this is due to the fact that
     the callback function cb_write_2() would need 2 parameters (one
     to indicate a boolean, another to indicate this condition),
     and can only get one. Then, we use a global indicator for this
     condition. */

  recv_buffers_t *recv_buffers;
  /* Next buffer that should contain incoming data. */
  int next_recv_buf;
  /* Current offset in the next buffer containing incoming data. */
  int next_recv_buf_offs;

  mdcp_param_info_t parameters;
  int main_chan_skip_size;
} mdcp_slots_info_t;

/*
 * mdcp_slots_info[] is indexed on CMEM_NSLOTS instead of CHANNEL_RANGE to
 * take less memory. Thus, an indirection throw mdcp_slots[] will be necessary
 * to access informations about a channel.
 */
static mdcp_slots_info_t mdcp_slots_info[CMEM_NSLOTS];

static int mdcp_slots[MAX_NODES - 1][CHANNEL_RANGE];

/*
 * We make intensive use of exclusive locks on the MDCP channels
 * because each operation of the MDCP layer uses several operations
 * of the underlying SLR/V layer. Since we may sleep, we must acquire
 * exclusive lock between the MDCP operations to get atomicity.
 *
 * We make use of the lock library imported from the MACH kernel VM.
 *
 * Note that we apply lock to the underlying main channel to lock a
 * MDCP channel.
 *
 * Since we want to exclude write operations, and read operations,
 * but we don't want to exclude write operations from read operations,
 * we have two locks for each channel.
 */

/* Exclusion between write operations. */
#ifndef _SMP_SUPPORT
lock_data_t channel_lock_out[MAX_NODES - 1][CHANNEL_RANGE];
#else
struct lock channel_lock_out[MAX_NODES - 1][CHANNEL_RANGE];
#endif
/* Exclusion between read operations. */
#ifndef _SMP_SUPPORT
lock_data_t channel_lock_in[MAX_NODES - 1][CHANNEL_RANGE];
#else
struct lock channel_lock_in[MAX_NODES - 1][CHANNEL_RANGE];
#endif

typedef struct _read_size_received_1 {
  size_t    size;
  boolean_t valid;
} read_size_received_1_t;

/* Wired down (not aligned) data for use by read/write functions. */
typedef struct _channel_wired_data {
  /* data protected with channel_lock_out */
  size_t write_size_sent_1;

  /* data protected with channel_lock_in */
  read_size_received_1_t read_size_received_1;  /* also a waiting condition */
} channel_wired_data_t;

channel_wired_data_t (*channel_wired_data)[MAX_NODES - 1]
                                          [CHANNEL_RANGE] = NULL;

/* Wired down constants. */
static const wired_const_NULL = 0;
/* static const wired_const_MDCP_RECV_BUF_SIZE = MDCP_RECV_BUF_SIZE; */

/* Declaration of local functions. */
static void cb_read_2 __P((caddr_t));

/*
 ************************************************************
 * mdcp_init() : initialize the MDCP layer.
 *
 * return values :  0 on success.
 *                 -1 on error.
 ************************************************************
 */

int
mdcp_init(void)
{
  int i, n;

  for (n = 0; n < MAX_NODES - 1; n++)
    for (i = 0; i < CHANNEL_RANGE; i++) {
      mdcp_slots[n][i] = -1;
#ifndef _SMP_SUPPORT
      lock_init(&channel_lock_in[n][i],  TRUE);
      lock_init(&channel_lock_out[n][i], TRUE);
#else
      lockinit(&channel_lock_in[n][i],  PVM, "mdcpini", 0, 0);
      lockinit(&channel_lock_out[n][i], PVM, "mdcpini", 0, 0);
#endif
    }

  if (channel_wired_data == NULL) {
    channel_wired_data = malloc(sizeof(*channel_wired_data), M_DEVBUF, M_WAITOK);
    if (channel_wired_data == NULL) return -1;
  }

  return 0;
}


/*
 ************************************************************
 * mdcp_end() : close the MDCP layer.
 *
 * return values :  0 on success.
 *                 -1 on error.
 ************************************************************
 */

int
mdcp_end(void)
{
  int i, n, res;
  int ret = 0;

  for (n = 0; n < MAX_NODES - 1; n++)
    for (i = 0; i < CHANNEL_RANGE; i++)
      if (mdcp_slots[n][i] != -1) {
	res = cmem_releasemem(mdcp_slots[n][i]);
	if (res < 0) {
	  log(LOG_ERR, "mdcp_end CMEM release error\n");
	  ret = -1;
	}
	mdcp_slots[n][i] = -1;
      }

  if (channel_wired_data) free(channel_wired_data, M_DEVBUF);
  channel_wired_data = NULL;
  return ret;
}


/*
 ************************************************************
 * mdcp_last_callback(param) : last callback function called at
 * channel close time.
 * param : corresponding slot.
 *
 * NOTE that this function may be called many times.
 ************************************************************
 */

boolean_t
mdcp_last_callback(caddr_t param)
{
  int slot, i, res;

  slot = (int) param;

  wakeup(&send_buffer_freed);
  wakeup(&(*channel_wired_data)[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                               [mdcp_slots_info[slot].parameters.main_channel].
	                       read_size_received_1);
  for (i = 0; i < MDCP_RECV_NBUFS; i++)
    wakeup(&(*mdcp_slots_info[slot].recv_buffers)[i].head);

#ifndef _SMP_SUPPORT
  res = lock_try_write(&channel_lock_out[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                                        [mdcp_slots_info[slot].parameters.main_channel]);
  if (res == FALSE) return FALSE;
#else
  res = lockmgr(&channel_lock_out[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                                 [mdcp_slots_info[slot].parameters.main_channel],
		LK_EXCLUSIVE | LK_NOWAIT, (void *) 0, curproc);
  if (res) return FALSE;
#endif

#ifndef _SMP_SUPPORT
  lock_done(&channel_lock_out[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                             [mdcp_slots_info[slot].parameters.main_channel]);
#else
  lockmgr(&channel_lock_out[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                           [mdcp_slots_info[slot].parameters.main_channel],
	  LK_RELEASE, (void *) 0, curproc);
#endif

#ifndef _SMP_SUPPORT
  res = lock_try_write(&channel_lock_in[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                                       [mdcp_slots_info[slot].parameters.main_channel]);
  if (res == FALSE) return FALSE;
#else
  res = lockmgr(&channel_lock_in[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                                [mdcp_slots_info[slot].parameters.main_channel],
		LK_EXCLUSIVE | LK_NOWAIT, (void *) 0, curproc);
  if (res) return FALSE;
#endif

#ifndef _SMP_SUPPORT
  lock_done(&channel_lock_in[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                            [mdcp_slots_info[slot].parameters.main_channel]);
#else
  lockmgr(&channel_lock_in[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                          [mdcp_slots_info[slot].parameters.main_channel],
	  	  LK_RELEASE, (void *) 0, curproc);
#endif
  return TRUE;
}


/*
 ************************************************************
 * mdcp_init_com(dest, chan1, chan2, classname, proc) :
 * associate an indirect buffer
 * with two channels.
 * dest         : remote node.
 * chan1, chan2 : pair of channels used by the protocol.
 * classname    : classname associtated with this pair of channels.
 * proc         : calling process.
 *
 * return values : 0 on success.
 *                 ERANGE    : channel or dest out of range.
 *                 EEXIST    : channel already initialized.
 *                 ENOMEM    : CMEM out of memory.
 *                 EFAULT    : an error occured while registering
 *                             pending receives or channel can't be opened.
 *                 ESHUTDOWN : channel is being shutdown.
 *                 EISCONN   : channel already opened.
 ************************************************************
 */

int
mdcp_init_com(dest, chan1, chan2, classname, proc)
              node_t         dest;
              channel_t      chan1, chan2;
	      appclassname_t classname;
	      struct proc   *proc;
{
  int slot;
  int ret;
  int i;

  if (chan1 >= CHANNEL_RANGE || chan2 >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan1] != -1) return EEXIST;

  ret = slrpv_open_channel(dest, chan1, classname, proc);
  if (ret) return ret;
  ret = slrpv_open_channel(dest, chan2, classname, proc);
  if (ret) {
    seq_t foo;
    slrpv_shutdown0_channel(dest, chan1, &foo, &foo, proc);
    slrpv_shutdown1_channel(dest, chan1, 0, 0, proc);
    slrpv_close_channel(dest, chan1, proc);
    return ret;
  }

  slot = cmem_getmem(sizeof(send_buffers_t) + sizeof(recv_buffers_t),
		     "MDCP");
  if (slot < 0) return ENOMEM;

  mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan1] = slot;

  bzero((caddr_t) cmem_virtaddr(slot),
	sizeof(send_buffers_t) + sizeof(recv_buffers_t));

  mdcp_slots_info[slot].send_buffers = (send_buffers_t *) cmem_virtaddr(slot);
  mdcp_slots_info[slot].recv_buffers = (recv_buffers_t *)
                                        (((void *) mdcp_slots_info[slot].send_buffers) +
                                        sizeof(send_buffers_t));

  mdcp_slots_info[slot].next_recv_buf      = 0;
  mdcp_slots_info[slot].next_recv_buf_offs = 0;

  mdcp_slots_info[slot].parameters.dest               = dest;
  mdcp_slots_info[slot].parameters.main_channel       = chan1;
  mdcp_slots_info[slot].parameters.bufferized_channel = chan2;

  mdcp_slots_info[slot].parameters.blocking                = TRUE;
  mdcp_slots_info[slot].parameters.input_buffering         = TRUE;
  mdcp_slots_info[slot].parameters.input_buffering_maxsize = 0;
  mdcp_slots_info[slot].parameters.output_buffering        = TRUE;

  mdcp_slots_info[slot].main_chan_skip_size = 0;

  slrpv_set_last_callback(dest, chan1, mdcp_last_callback, (caddr_t) slot, proc);
  slrpv_set_last_callback(dest, chan2, NULL, NULL, proc);

  /* Prepare RECEIVEs on the bufferized channel. */
  for (i = 0; i < MDCP_RECV_NBUFS; i++) {
    (*mdcp_slots_info[slot].recv_buffers)[i].head.in_use = TRUE;
    ret = slrpv_recv_piggy_back(dest, chan2,
				(caddr_t) &(*mdcp_slots_info[slot].recv_buffers)[i].head.size,
				sizeof((*mdcp_slots_info[slot].recv_buffers)[i].head.size),
				(*mdcp_slots_info[slot].recv_buffers)[i].data,
				MDCP_RECV_BUF_SIZE,
				cb_read_2,
				/* (caddr_t) &(*mdcp_slots_info[slot].recv_buffers)[i].head */
				(caddr_t) ((i<<16) | slot),
				proc);
    if (ret) return EFAULT;
  }

  /* Should not be useful, but if a preceding usage of this pair of channels
   was not clean, this can avoid some unwanted freeze... */
#ifndef _SMP_SUPPORT
  lock_init(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][chan1], TRUE);
  lock_init(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][chan1], TRUE);
  lock_init(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][chan2], TRUE);
  lock_init(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][chan2], TRUE);
#else
  lockinit(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][chan1], PVM, "mdcpini", 0, 0);
  lockinit(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][chan1], PVM, "mdcpini", 0, 0);
  lockinit(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][chan2], PVM, "mdcpini", 0, 0);
  lockinit(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][chan2], PVM, "mdcpini", 0, 0);
#endif

  /* Only the main channel is in charge of the select() operation. */
  channel_data_ready[NODE2NODEINDEX(dest, my_node)][chan1] = DATA_READY_OU;
  bzero(&channel_select_info_in[NODE2NODEINDEX(dest, my_node)][chan1],
	sizeof(struct selinfo));
  bzero(&channel_select_info_ou[NODE2NODEINDEX(dest, my_node)][chan1],
	sizeof(struct selinfo));
  bzero(&channel_select_info_ex[NODE2NODEINDEX(dest, my_node)][chan1],
	sizeof(struct selinfo));

  return 0;
}


/*
 ************************************************************
 * mdcp_shutdown0_com(dest, chan, seq_send_0, seq_recv_0, seq_send_1, seq_recv_1, proc) :
 * shutdown a couple of channels.
 * dest       : remote node.
 * chan       : channel representing the slot.
 * seq_send_0 : variable ready to contain sequence number to reach.
 * seq_recv_0 : variable ready to contain sequence number to reach.
 * seq_send_1 : variable ready to contain sequence number to reach.
 * seq_recv_1 : variable ready to contain sequence number to reach.
 * proc       : proc structure of the calling process.
 *
 * return values : 0 on success.
 *                 ERANGE    : dest or channel out of range.
 *                 ENOENT    : channel not initialized.
 *                 ENOMEM    : CMEM was unabled to release memory.
 *                 ESHUTDOWN : channel is being shutdown.
 *                 ENOTCONN  : channel closed.
 *                 EFAULT    : channel can't be shutdown.
 ************************************************************
 */

int
mdcp_shutdown0_com(dest, chan, seq_send_0, seq_recv_0, seq_send_1, seq_recv_1, proc)
                   node_t dest;
		   channel_t chan;
		   seq_t *seq_send_0, *seq_recv_0, *seq_send_1, *seq_recv_1;
		   struct proc *proc;
{
  int res;
  int slot;
  mdcp_slots_info_t *slot_info;

  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;

  /* WE MUST NOT get atomicity on this channel : the channel MAY already
     be locked. */

  slot = mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan];
  if (slot == -1) return ENOENT;

  slot_info = mdcp_slots_info + slot;

  res = slrpv_shutdown0_channel(dest, slot_info->parameters.main_channel,
				seq_send_0, seq_recv_0, proc);
  if (res) return res;

  res = slrpv_shutdown0_channel(dest, slot_info->parameters.bufferized_channel,
				seq_send_1, seq_recv_1, proc);
  if (res) return res;

  return 0;
}


/*
 ************************************************************
 * mdcp_shutdown1_com(dest, chan, seq_send_0, seq_recv_0, seq_send_1, seq_recv_1, proc) :
 * shutdown a couple of channels.
 * dest       : remote node.
 * chan       : channel representing the slot.
 * seq_send_0 : max sequence number to reach.
 * seq_recv_0 : max sequence number to reach.
 * seq_send_1 : max sequence number to reach.
 * seq_recv_1 : max sequence number to reach.
 * proc       : proc structure of the calling process.
 *
 * return values : 0 on success.
 *                 ERANGE    : dest or channel out of range.
 *                 ENOENT    : channel not initialized.
 *                 ENOMEM    : CMEM was unabled to release memory.
 *                 ESHUTDOWN : channel is being shutdown.
 *                 ENOTCONN  : channel closed.
 *                 EFAULT    : channel can't be shutdown.
 *                 EISCONN   : channel still opened.
 *                 ...       : slrp_send/recv() error codes.
 ************************************************************
 */

int
mdcp_shutdown1_com(dest, chan, seq_send_0, seq_recv_0, seq_send_1, seq_recv_1, proc)
                   node_t dest;
		   channel_t chan;
		   seq_t seq_send_0, seq_recv_0, seq_send_1, seq_recv_1;
		   struct proc *proc;
{
  int res;
  int slot;
  mdcp_slots_info_t *slot_info;

  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;

  /* WE MUST NOT get atomicity on this channel : the channel MAY already
     be locked. */

  slot = mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan];
  if (slot == -1) return ENOENT;

  slot_info = mdcp_slots_info + slot;

  res = slrpv_shutdown1_channel(dest, slot_info->parameters.main_channel,
				seq_send_0, seq_recv_0, proc);
  if (res) return res;

  res = slrpv_shutdown1_channel(dest, slot_info->parameters.bufferized_channel,
				seq_send_1, seq_recv_1, proc);
  if (res) return res;

  return 0;
}


/*
 ************************************************************
 * mdcp_end_com(dest, chan, proc) : free the slot of memory
 * associated with a channel.
 * dest : remote node.
 * chan : channel representing the slot.
 * proc : proc structure of the calling process.
 *
 * return values : 0 on success.
 *                 ERANGE    : dest or channel out of range.
 *                 ENOENT    : channel not initialized.
 *                 ENOMEM    : CMEM was unabled to release memory.
 *                 ESHUTDOWN : channel is being shutdown.
 *                 ENOTCONN  : channel already closed.
 *                 EISCONN   : channel opened.
 *                 EFAULT    : channel can't be closed.
 *                 EAGAIN    : channel being closed, try again to check
 *                             for completion.
 ************************************************************
 */

int
mdcp_end_com(dest, chan, proc)
             node_t dest;
	     channel_t chan;
	     struct proc *proc;
{
  int s;
  int res;
  int slot;
  mdcp_slots_info_t *slot_info;

  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;

  /* WE MUST NOT get atomicity on this channel : the channel MAY already
     be locked. */

  slot = mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan];
  if (slot == -1) return ENOENT;

  slot_info = mdcp_slots_info + slot;

  res = slrpv_close_channel(dest, slot_info->parameters.main_channel, proc);
  if (res && res != ENOTCONN) return res;

  res = slrpv_close_channel(dest, slot_info->parameters.bufferized_channel, proc);
  if (res && res != ENOTCONN) return res;

  res = cmem_releasemem(mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan]);
  if (res < 0) return ENOMEM;

  mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan] = -1;

  /* Wake-up any process waiting on a select() operation. */
  s = splhigh();
  channel_data_ready[NODE2NODEINDEX(slot_info->parameters.dest, my_node)]
                    [slot_info->parameters.main_channel] |= DATA_READY_IN;
  selwakeup(&channel_select_info_in[NODE2NODEINDEX(slot_info->parameters.dest, my_node)]
                                   [slot_info->parameters.main_channel]);
  splx(s);

  return 0;
}


/*
 ************************************************************
 * mdcp_getparams(dest, chan, param) : get parameters.
 * dest  : remote node;
 * chan  : channel affected.
 * param : pointer to parameters buffer.
 *
 * return values : 0 on success.
 *                 ERANGE : dest or channel out of range.
 *                 ENOENT : channel not initialized.
 ************************************************************
 */

int
mdcp_getparams(dest, chan, param)
               node_t             dest;
               channel_t          chan;
               mdcp_param_info_t *param;
{
  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan] == -1) return ENOENT;

  bcopy(&mdcp_slots_info[mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan]].parameters,
	param, sizeof *param);

  return 0;
}


/*
 ************************************************************
 * mdcp_setparam_blocking(dest, chan, param) : set blocking parameter.
 * dest  : remote node.
 * chan  : channel affected.
 * param : blocking/non blocking behaviour.
 *
 * return values : 0 on success.
 *                 ERANGE : dest or channel out of range.
 *                 ENOENT : channel not initialized.
 ************************************************************
 */

int
mdcp_setparam_blocking(dest, chan, param)
                       node_t    dest;
                       channel_t chan;
                       boolean_t param;
{
  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[dest][chan] == -1) return ENOENT;

  mdcp_slots_info[mdcp_slots[NODE2NODEINDEX(dest, my_node)]
		 [chan]].parameters.blocking = param;

  return 0;
}


/*
 ************************************************************
 * mdcp_setparam_input_buffering(dest, chan, param) : set input buffering.
 * dest  : remote node.
 * chan  : channel affected.
 * param : input buffering.
 *
 * return values : 0 on success.
 *                 ERANGE : dest or channel out of range.
 *                 ENOENT : channel not initialized.
 ************************************************************
 */

int
mdcp_setparam_input_buffering(dest, chan, param)
                              node_t dest;
                              channel_t chan;
                              boolean_t param;
{
  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan] == -1) return ENOENT;

  mdcp_slots_info[mdcp_slots[NODE2NODEINDEX(dest, my_node)]
		 [chan]].parameters.input_buffering = param;

  return 0;
}


/*
 ************************************************************
 * mdcp_setparam_input_buffering_maxsize(dest, chan, param) : set input
 * buffering max size.
 * dest  : remote node.
 * chan  : channel affected.
 * param : max size for input buffering.
 *
 * return values : 0 on success.
 *                 ERANGE : dest or channel out of range.
 *                 ENOENT : channel not initialized.
 ************************************************************
 */

int
mdcp_setparam_input_buffering_maxsize(dest, chan, param)
                                      node_t    dest;
                                      channel_t chan;
                                      size_t    param;
{
  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan] == -1) return ENOENT;

  mdcp_slots_info[mdcp_slots[NODE2NODEINDEX(dest, my_node)]
		 [chan]].parameters.input_buffering_maxsize = param;

  return 0;
}


/*
 ************************************************************
 * mdcp_setparam_output_buffering(dest, chan, param) : set output buffering.
 * dest  : remote node;
 * chan  : channel affected.
 * param : output buffering.
 *
 * return values : 0 on success.
 *                 ERANGE : dest or channel out of range.
 *                 ENOENT : channel not initialized.
 ************************************************************
 */

int
mdcp_setparam_output_buffering(dest, chan, param)
                               node_t    dest;
                               channel_t chan;
                               boolean_t param;
{
  if (chan >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (mdcp_slots[NODE2NODEINDEX(dest, my_node)][chan] == -1) return ENOENT;

  mdcp_slots_info[mdcp_slots[NODE2NODEINDEX(dest, my_node)]
		 [chan]].parameters.output_buffering = param;

  return 0;
}


/*
 ************************************************************
 * cb_write_1(param, size) : 1st callback function for mdcp_write().
 * param : variable ready to receive size sent.
 * size  : size sent.
 ************************************************************
 */

static void
cb_write_1(param, size)
           caddr_t param;
           int     size;
{
  /* We should be at splhigh processor level here. */

  *(size_t *) param = size;
  wakeup(param);
}


/*
 ************************************************************
 * cb_write_2(param, size) : 2nd callback function for mdcp_write().
 * param  : buffer to release.
 * bufnum : size sent.
 ************************************************************
 */

static void
cb_write_2(param, size)
           caddr_t param;
           int     size;
{
  /* We should be at splhigh processor level here. */

  /* Release the sending buffer. */
  *(boolean_t *) param = FALSE;

  /* Wake-up processes waiting for a sending buffer. */
  wakeup(&send_buffer_freed);
}


/*
 ************************************************************
 * mdcp_write(dest, channel, buf, size, ret_size, proc) : write data
 * on a MDCP channel.
 *
 * dest     : destination node.
 * channel  : main channel to write on.
 * buf      : local user data.
 * size     : size of local user data.
 * ret_size : variable receiving the size sent.
 * proc     : process asking this job.
 *
 * return values : 0 on success.
 *                 ERANGE      : parameters out of range.
 *                 ENOENT      : channel not initialized or this board
 *                               not initialized.
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized or not enough
 *                               structures allocated at compilation
 *                               time to handle this board.
 *                 EINVAL      : size is NULL.
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 EFAULT      : a bad address is encountered,
 *                               or an unrecoverable error,
 *                 EIO         : no mapping for some pages.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 *                 EMSGSIZE    : size too big.
 ************************************************************
 */

int
mdcp_write(dest, channel, buf, size, ret_size, proc)
           node_t       dest;
           channel_t    channel;
	   caddr_t      buf;
	   size_t       size;
	   size_t      *ret_size;
	   struct proc *proc;
{
  int s, i;
  int slot;
  mdcp_slots_info_t *slot_info;
  int ret, res;

  if (channel >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (buf == NULL || ret_size == NULL || size == 0L) return ERANGE;
  if (size > MDCP_RECV_BUF_SIZE) return EMSGSIZE;

  /* Get atomicity on this channel. */
#ifndef _SMP_SUPPORT
  lock_write(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
  lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	  LK_EXCLUSIVE | LK_RETRY, (void *) 0, proc);
#endif

  /* Each time we may have sleepped on a condition that is raised at close time,
     we must check that the channel is still opened. */
  if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
    *ret_size = 0;
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;
  }

  slot = mdcp_slots[NODE2NODEINDEX(dest, my_node)][channel];
  if (slot == -1) {
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return ENOENT;
  }

  /* vérifier qu'on risque pas d'être bloqué (cf blocking) */

  slot_info = mdcp_slots_info + slot;

  /* Is the local buffer big enough ? */
  if (slot_info->parameters.input_buffering == TRUE &&
      size > MDCP_SEND_BUF_SIZE &&
      (!slot_info->parameters.input_buffering_maxsize ||
       slot_info->parameters.input_buffering_maxsize > MDCP_SEND_BUF_SIZE)) {
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return EMSGSIZE;
  }

  s = splhigh();

  /* Can we send data without any buffer copy ? */
  if (slrpv_cansend(dest, channel) == TRUE) {
    /* Yes, there is a pending receive on the main channel. */

    /*
     * Send data on the main channel.
     * write_size_sent_1 is a R/W variable : the callback function stores in
     * this variable the size really sent. write_size_sent_1 is also a waiting
     * condition.
     */
    (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1 = size;

    /*
     * NOTE THAT we give the address of data associated with a channel
     * (channel_wired_data) :
     * we must not exit this function before completion of the following call.
     */
    ret = slrpv_send_piggy_back_prot(dest, channel,
				     (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1,
				     sizeof((*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1),
				     buf, size,
				     cb_write_1,
				     (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1,
				     proc);
    if (ret) {
      log(LOG_ERR, "FATAL ERROR: mdcp_write 1\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    /*
     * Wait for completion.
     * Note that we do not use flag PCATCH : we assume that the completion
     * will happen soon.
     */
    ret = tsleep(&(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
		                       [channel].write_size_sent_1,
		 HSLPRI, "MDCPW1", NULL);

    /* Each time we may have sleepped on a condition that is raised at close time,
       we must check that the channel is still opened. */
    if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
      *ret_size = 0;
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return 0;
    }

    if (ret) {
      log(LOG_ERR, "FATAL ERROR: mdcp_write 2\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    *ret_size = (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                     [channel].write_size_sent_1 - 4;

    splx(s);
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;
  }

  /************************************************************/

  /* There is no pending receive on the main channel,
     we may do input/output buffering. */

  if (slot_info->parameters.input_buffering == TRUE &&
      (!slot_info->parameters.input_buffering_maxsize ||
       slot_info->parameters.input_buffering_maxsize >= size)) {
    /*
     * Input buffering. Input buffering implies output buffering too, even
     * if the user did not ask for it.
     */

  restart:
    /* Find a free sending buffer. */
    for (i = 0; i < MDCP_SEND_NBUFS; i++)
      if ((*slot_info->send_buffers)[i].head.in_use == FALSE) break;
    if (i == MDCP_SEND_NBUFS) {
      /*
       * Wait for a free buffer.
       */
      ret = tsleep(/* &slot_info->send_buffers */ &send_buffer_freed,
      HSLPRI | PCATCH, "MDCPW2", NULL);

      /* Each time we may have sleepped on a condition that is raised at close time,
	 we must check that the channel is still opened. */
      if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
	*ret_size = 0;
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return 0;
      }

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 3\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      goto restart;
    }

    /* Lock the sending buffer. */
    (*slot_info->send_buffers)[i].head.in_use = TRUE;

    /* Copy data into the sending buffer. */
    res =  copyin(buf, (*slot_info->send_buffers)[i].data, size);
    if (res) {
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return res;
    }

    /* Send data : we don't need to wait for the completion of this call
       since size_sent is part of the buffer head. */
    (*slot_info->send_buffers)[i].head.size_sent = size;    
    ret = slrpv_send_piggy_back(dest,
				slot_info->parameters.bufferized_channel,
				(caddr_t) &(*slot_info->send_buffers)[i].head.size_sent,
				sizeof((*slot_info->send_buffers)[i].head.size_sent),
				(*slot_info->send_buffers)[i].data, size,
				cb_write_2,
				(caddr_t) &(*slot_info->send_buffers)[i].head.in_use,
				proc);

    if (ret) {
      log(LOG_ERR, "FATAL ERROR: mdcp_write 4\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    /*
     * Send a zero length message on the main channel.
     * Note that there is certainly no pending receive for this message,
     * its completion may not happen soon.
     */
    ret = slrpv_send(dest, channel, (caddr_t) &wired_const_NULL,
		     sizeof(wired_const_NULL), NULL, NULL, proc);

    if (ret) {
      log(LOG_ERR, "FATAL ERROR: mdcp_write 5\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    /*
     * We do input buffering not to wait for completion of the first message.
     * Then we must determine the size really sent BEFORE the completion
     * of the first message. We can do this now, because we know the size of
     * the RECEIVE messages on the bufferized channel.
     */
    *ret_size = MIN(size, MDCP_RECV_BUF_SIZE);

    splx(s);
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;

  } else {
    /************************************************************/

    /* No input buffering. */

    /* Output buffering ? */
    if (slot_info->parameters.output_buffering == TRUE) {
      /* Yes, output buffering (and no input buffering). */

      /*
       * Send data on the bufferized channel.
       * write_size_sent is a R/W variable : the callback function stores in
       * this variable the size really sent. write_size_sent is also a waiting
       * condition.
       */
      (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1 = size;

      /*
       * NOTE THAT we give the address of data associated with a channel
       * (channel_wired_data) :
       * we must not exit this function before completion of the following call.
       */
      ret = slrpv_send_piggy_back_prot(dest,
				       slot_info->parameters.bufferized_channel,
				       (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1,
				       sizeof((*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1),
				       buf, size,
				       cb_write_1,
				       (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1,
				       proc);

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 6\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      /*
       * Send a zero length message on the main channel.
       * Note that there is certainly no pending receive for this message,
       * its completion may not happen soon.
       */
      ret = slrpv_send(dest, channel,
		       (caddr_t) &wired_const_NULL, sizeof(wired_const_NULL),
		       NULL, NULL, proc);

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 7\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      /*
       * Wait for completion of the message to get the data size really sent.
       * Note that we do not use flag PCATCH : we assume that the completion
       * will happen soon.
       */
      ret = tsleep(&(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
		                         [channel].write_size_sent_1,
		   HSLPRI, "MDCPW3", NULL);

      /* Each time we may have sleepped on a condition that is raised at close time,
	 we must check that the channel is still opened. */
      if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
	*ret_size = 0;
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return 0;
      }

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 8\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      /* Update the returned size. */
      *ret_size = (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
	                               [channel].write_size_sent_1 - 4;

      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return 0;

    } else {
      /* No output buffering (and no input buffering). */

      /*
       * Send data on the main channel.
       * write_size_sent is a R/W variable : the callback function stores in
       * this variable the size really sent. write_size_sent is also a waiting
       * condition.
       */
      (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].write_size_sent_1 = size;

      /*
       * NOTE THAT we give the address of data associated with a channel
       * (channel_wired_data) :
       * we must not exit this function before completion of the following call.
       */
      ret = slrpv_send_piggy_back_prot(dest, channel,
				       (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                                             [channel].write_size_sent_1,
				       sizeof((*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                                                   [channel].write_size_sent_1),
				       buf, size,
				       cb_write_1,
				       (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                                                       [channel].write_size_sent_1,
				       proc);

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 9\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      /*
       * Wait for completion.
       * Note that we do not use flag PCATCH : we assume that the completion
       * will happen soon.
       */
      ret = tsleep(&(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
		                         [channel].write_size_sent_1,
		   HSLPRI, "MDCPW4", NULL);

      /* Each time we may have sleepped on a condition that is raised at close time,
	 we must check that the channel is still opened. */
      if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
	*ret_size = 0;
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return 0;
      }

      if (ret) {
	log(LOG_ERR, "FATAL ERROR: mdcp_write 10\n");
	splx(s);
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return ret;
      }

      *ret_size = (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
	                               [channel].write_size_sent_1 - 4;

      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return 0;
    }
  }

  /* Should not happen... */
  log(LOG_ERR, "mdcp_write: error\n");

  splx(s);
#ifndef _SMP_SUPPORT
  lock_done(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel]);
#else
  lockmgr(&channel_lock_out[NODE2NODEINDEX(dest, my_node)][channel],
	  LK_RELEASE, (void *) 0, proc);
#endif
  return EFAULT;
}


/*
 ************************************************************
 * cb_read_1(param) : 1st callback function for mdcp_read().
 * param : variable ready to receive size received.
 ************************************************************
 */

static void
cb_read_1(param)
          caddr_t param;
{
  /* We should be at splhigh processor level here. */

  ((read_size_received_1_t *) param)->valid = TRUE;
  wakeup(param);
}


/*
 ************************************************************
 * cb_read_2(param) : 2nd callback function for mdcp_read() :
 * handle receipt on the bufferized channel.
 * param : pointer to the buffer head.
 ************************************************************
 */

static void
cb_read_2(param)
          caddr_t param;
{
  int slot, nbuf;

  /* We should be at splhigh processor level here. */

  slot = ((int) param) & 0xFFFF;
  nbuf = ((int) param)>>16;

  (*mdcp_slots_info[slot].recv_buffers)[nbuf].head.in_use = FALSE;
  wakeup(&(*mdcp_slots_info[slot].recv_buffers)[nbuf].head);

  channel_data_ready[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                    [mdcp_slots_info[slot].parameters.main_channel] |= DATA_READY_IN;
  selwakeup(&channel_select_info_in[NODE2NODEINDEX(mdcp_slots_info[slot].parameters.dest, my_node)]
                                   [mdcp_slots_info[slot].parameters.main_channel]);
  /*
  ((mdcp_recv_head_t *) param)->in_use = FALSE;
  wakeup(param);
  */
}

/*
 ************************************************************
 * mdcp_read(dest, channel, buf, size, ret_size, proc) : read data
 * from a MDCP channel.
 *
 * dest     : destination node.
 * channel  : main channel to write on.
 * buf      : local user data.
 * size     : size of local user data.
 * ret_size : variable receiving the size sent.
 * proc     : process asking this job.
 *
 * return values : 0           : success.
 *                 ERANGE      : parameters out of range.
 *                 ENOENT      : channel not initialized or this board
 *                               not initialized.
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 ENXIO       : not enough structures allocated at
 *                             :  compilation time to handle this board.
 *                 ENOENT      : this board is not initialized.
 *                 EIO         : no mapping for some pages.
 *                 EACCES      : not enough rights.
 *                 ENOMEM      : size too big.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 *                 EMSGSIZE    : size too big.
 ************************************************************
 */

int
mdcp_read(dest, channel, buf, size, ret_size, proc)
          node_t       dest;
	  channel_t    channel;
	  caddr_t      buf;
	  size_t       size;
	  size_t      *ret_size;
	  struct proc *proc;
{
  int slot;
  mdcp_slots_info_t *slot_info;
  int ret, res;
  int s;

  if (channel >= CHANNEL_RANGE) return ERANGE;
  if (dest >= MAX_NODES) return ERANGE;
  if (buf == NULL || ret_size == NULL || size == 0L) return ERANGE;
  if (size > MDCP_RECV_BUF_SIZE) return EMSGSIZE;

  /* Get atomicity on this channel. */
#ifndef _SMP_SUPPORT
  lock_write(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
  lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	  LK_EXCLUSIVE | LK_RETRY, (void *) 0, proc);
#endif

  /* Each time we may have sleepped on a condition that is raised at close time,
     we must check that the channel is still opened. */
  if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
    *ret_size = 0;
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;
  }

  slot = mdcp_slots[NODE2NODEINDEX(dest, my_node)][channel];
  if (slot == -1) {
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return ENOENT;
  }

  /* vérifier qu'on risque pas d'être bloqué (cf blocking) */

  slot_info = mdcp_slots_info + slot;

  s = splhigh();

#if 0
  /* THIS OPTIMIZATION WILL NEED MODIFICATIONS IN SLR/P
     -> not yet implemented */

  /* Do we have valid data in a local buffer ? */
  if ((*slot_info->recv_buffers)[slot_info->next_recv_buf].head.in_use == FALSE) {
    /* Yes, local valid data present. */

    /*
     * Important optimization : we don't receive on the main channel since
     * we know a piggy backed message of NULL size would be received.
     * To save this RECEIVE operation, we remember this information in order
     * to have the next RECEIVE operation to read the NULL size data at
     * the same time that any other data. This is just an aggregation of several
     * RECEIVE operations at the same time.
     */

    slot_info->main_chan_skip_size += sizeof(size_t);

    (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                         [channel].read_size_received_1.size = 0;

  } else {

    /* Receive data from the main channel. */
    /*
     * NOTE THAT we give the address of data associated with a channel
     * (channel_wired_data) :
     * we must not exit this function before completion of the following call.
     */
    (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                         [channel].read_size_received_1.valid = FALSE;


    /* To be written : RECEIVE DATA SKIPPING SOME BYTES */
  }
#endif

  if (!slot_info->next_recv_buf_offs) {
    /* We don't have a local receive buffer partially read. */

    (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                         [channel].read_size_received_1.valid = FALSE;

    ret = slrpv_recv_piggy_back_prot(dest, channel,
				     (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].read_size_received_1.size,
				     sizeof((*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].read_size_received_1.size),
				     buf, size, cb_read_1,
				     (caddr_t) &(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)][channel].read_size_received_1,
				     proc);

    if (ret) {
      log(LOG_ERR, "FATAL ERROR: mdcp_read 1\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    /* Wait for completion. */
  restart:
    if ((*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
	[channel].read_size_received_1.valid == FALSE) {
      ret = tsleep(&(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                         [channel].read_size_received_1,
		   HSLPRI | PCATCH, "MDCPR1", NULL);

      /* Each time we may have sleepped on a condition that is raised at close time,
	 we must check that the channel is still opened. */
      if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
	*ret_size = 0;
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return 0;
      }

      if (!ret) goto restart; /* TRY AGAIN */

      log(LOG_ERR, "FATAL ERROR: mdcp_read 2\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }
  }

  /************************************************************/

  /* Are there some data in the local buffers ? */
  if (slot_info->next_recv_buf_offs ||
      !(*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                            [channel].read_size_received_1.size) {
    /* Yes, we must fetch data from a local buffer. */

    /*
     * Wait for valid data in the buffer. Since we may wait for a while, we use
     * flag PCATCH.
     */
   restart2:
    if ((*slot_info->recv_buffers)[slot_info->next_recv_buf].head.in_use == TRUE) {
      /* No data in the buffer. */

      ret = tsleep(&(*slot_info->recv_buffers)[slot_info->next_recv_buf].head,
		   HSLPRI | PCATCH, "MDCPR2", NULL);

      /* Each time we may have sleepped on a condition that is raised at close time,
	 we must check that the channel is still opened. */
      if (channel_state[NODE2NODEINDEX(dest, my_node)][channel] != CHAN_OPEN) {
	*ret_size = 0;
#ifndef _SMP_SUPPORT
	lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
	lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
		LK_RELEASE, (void *) 0, proc);
#endif
	return 0;
      }

      if (!ret) goto restart2;

      log(LOG_ERR, "FATAL ERROR: mdcp_read 3\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return ret;
    }

    /* Copy data into the user space. */
    res = copyout((*slot_info->recv_buffers)[slot_info->next_recv_buf].data +
		  slot_info->next_recv_buf_offs,
		  buf,
		  MIN(size,
		      (*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size -
		      slot_info->next_recv_buf_offs));

    if (res) {
      log(LOG_ERR, "FATAL ERROR: mdcp_read 4\n");
      splx(s);
#ifndef _SMP_SUPPORT
      lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
      lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	      LK_RELEASE, (void *) 0, proc);
#endif
      return res;
    }

    /* Update the returned size. */
    *ret_size = MIN(size,
		    (*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size -
		    slot_info->next_recv_buf_offs);

    /* Update the local buffer. */
    if ((*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size
	- slot_info->next_recv_buf_offs > size) {
      /* The buffer is not empty. */
      slot_info->next_recv_buf_offs +=
	MIN(size, (*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size -
	          slot_info->next_recv_buf_offs);
    } else {
      /* The buffer is empty. */

      /* Prepare another RECEIVE on the bufferized channel. */
      (*slot_info->recv_buffers)[slot_info->next_recv_buf].head.in_use = TRUE;
      ret = slrpv_recv_piggy_back(dest,
				  slot_info->parameters.bufferized_channel,
				  (caddr_t) &(*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size,
				  sizeof((*slot_info->recv_buffers)[slot_info->next_recv_buf].head.size),
				  (*slot_info->recv_buffers)[slot_info->next_recv_buf].data,
				  MDCP_RECV_BUF_SIZE,
				  cb_read_2,
				  /* (caddr_t) &(*mdcp_slots_info[slot].recv_buffers)[slot_info->next_recv_buf].head */
				  (caddr_t) ((slot_info->next_recv_buf<<16) | slot),
				  proc);
      if (ret) return EFAULT;

      /* Increment next buffer. */
      if (++slot_info->next_recv_buf == MDCP_RECV_NBUFS)
	slot_info->next_recv_buf = 0;
      slot_info->next_recv_buf_offs = 0;

      /* Do we have some pending data in the local buffers ? */
      if ((*slot_info->recv_buffers)[slot_info->next_recv_buf].head.in_use == TRUE)
	/* No, update this information for the select operation. */
	channel_data_ready[NODE2NODEINDEX(slot_info->parameters.dest, my_node)]
                          [slot_info->parameters.main_channel] &= ~DATA_READY_IN;
    }

    splx(s);
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;

  } else {
    /* No data in a local buffer. Then data must have been read during the RECEIVE at
     the beginning of this function. */

    /* Update the return size. */
    *ret_size = (*channel_wired_data)[NODE2NODEINDEX(dest, my_node)]
                                     [channel].read_size_received_1.size;

    splx(s);
#ifndef _SMP_SUPPORT
    lock_done(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel]);
#else
    lockmgr(&channel_lock_in[NODE2NODEINDEX(dest, my_node)][channel],
	    LK_RELEASE, (void *) 0, proc);
#endif
    return 0;
  }
}


/******************************************************************************
 * HSL SELECT IMPLEMENTATION derivated from the BSD4.4 select implementation. *
 ******************************************************************************/

/* /sys/kern/tty_pty.c is a good example to understand the use of
   selinfo/selrecord()/selwakeup() */

extern int nselcoll;
extern int selwait;

#ifndef _SMP_SUPPORT

static int selscan __P((struct proc *, fd_mask **, fd_mask **, int, int *));
static int chanscan __P((struct proc *,
			 mpc_chan_set *, mpc_chan_set *, mpc_chan_set *, int *));

int
hsl_select(p, uap, retval)
           register struct proc *p;
	   register libmpc_select_t *uap;
	   int *retval;
{
  fd_mask *ibits[3], *obits[3];
  struct timeval atv;
  int s, ncoll, error = 0, timo, i, max_idx;
  u_int ni;

  mpc_chan_set *chanset_in = NULL;
  mpc_chan_set *chanset_ou = NULL;
  mpc_chan_set *chanset_ex = NULL;

  if (uap->nd < 0)
    return EINVAL;

  if ((uap->mpcchanset_in != NULL && (uap->mpcchanset_in_max_index < 0 ||
				      uap->mpcchanset_in_max_index >= MAX_CHAN_PROC)) ||
      (uap->mpcchanset_ou != NULL && (uap->mpcchanset_ou_max_index < 0 ||
				      uap->mpcchanset_ou_max_index >= MAX_CHAN_PROC)) ||
      (uap->mpcchanset_ex != NULL && (uap->mpcchanset_ex_max_index < 0 ||
				      uap->mpcchanset_ex_max_index >= MAX_CHAN_PROC)))
    return EINVAL;

  if (uap->nd > p->p_fd->fd_nfiles)
    uap->nd = p->p_fd->fd_nfiles;   /* forgiving; slightly wrong */

  /* The amount of space we need to allocate */
  ni = howmany(roundup2(uap->nd, FD_SETSIZE), NFDBITS) *
    sizeof(fd_mask);

  /*
   * The malloc for file descriptors bits is often performed only one time for
   * a process. The free is only done at exit time (/sys/kern/kern_exit.c) or
   * is the allocated space is not big enough.
   *
   * Doing the same way with the channel bits would drastically improve
   * the performances -> THIS OPTIMIZATION SHOULD BE DONE.
   * But it is not an easy work because we do not want to modify the definition
   * of the kernel proc structure: if so, it would require to have a particular
   * kernel to run MPC-OS.
   */
  if (ni > p->p_selbits_size) {
    if (p->p_selbits_size)
      free (p->p_selbits, M_SELECT);

    while (p->p_selbits_size < ni)
      p->p_selbits_size += 32; /* Increase by 256 bits */

    p->p_selbits = malloc(p->p_selbits_size * 6, M_SELECT,
			  M_WAITOK);
  }

  /*
   * ibits contains the bits set for input, output and exceptionnal
   * conditions handling.  ibits and obits are passed to selscan() that fills
   * obits to indicate wether some input, output or exceptionnal conditions
   * happened.
   */
  for (i = 0; i < 3; i++) {
    ibits[i] = (fd_mask *)(p->p_selbits + i * p->p_selbits_size);
    obits[i] = (fd_mask *)(p->p_selbits + (i + 3) *
			   p->p_selbits_size);
  }

  /*
   * This buffer is usually very small therefore it's probably faster
   * to just zero it, rather than calculate what needs to be zeroed.
   */
  bzero(p->p_selbits, p->p_selbits_size * 6);

  /* The amount of space we need to copyin/copyout */
  ni = howmany(uap->nd, NFDBITS) * sizeof(fd_mask);

#define	getbits(name, x) \
	if (uap->name && \
	    (error = copyin((caddr_t)uap->name, (caddr_t)ibits[x], ni))) \
		goto done;
  getbits(in, 0);
  getbits(ou, 1);
  getbits(ex, 2);
#undef	getbits

  /* Get the channel bit sets. */
  /* DOING ONLY ONE MALLOC instead of 3 would improve the performances. */
#define GET_CHAN_BITS(TYPE) \
  if (uap->mpcchanset_##TYPE != NULL) { \
    chanset_##TYPE = (mpc_chan_set *) malloc(sizeof(int) + \
                     sizeof(mpc_chan_set_t) * (uap->mpcchanset_##TYPE##_max_index + 1), \
                     M_SELECT, M_WAITOK); \
    error = copyin((caddr_t) uap->mpcchanset_##TYPE, chanset_##TYPE, sizeof(int) + \
                   sizeof(mpc_chan_set_t) * (uap->mpcchanset_##TYPE##_max_index + 1)); \
    if (error) goto done; \
    /* Applying basic security rules: the user could have given a different max_index in mpcchanset_in_max_index than in the mpc_chan_set structure. */ \
    if (chanset_##TYPE->max_index != uap->mpcchanset_##TYPE##_max_index) \
        log(LOG_ERR, "hsl_select(): invalid index\n"); \
    chanset_##TYPE->max_index = uap->mpcchanset_##TYPE##_max_index; \
    for (i = 0; i <= chanset_##TYPE->max_index; i++) \
        chanset_##TYPE->chan_set[i].data_ready = 0; \
  }
  GET_CHAN_BITS(in);
  GET_CHAN_BITS(ou);
  GET_CHAN_BITS(ex);
#undef GET_CHAN_BITS

  /* SHOULD BE CHANGED : we do it like if write was always possible on any channel
     (it is easier to code, and I hope it to be a not so bad heuristic...). */
  if (chanset_ou != NULL)
    for (i = 0; i <= chanset_ou->max_index; i++)
      if (chanset_ou->chan_set[i].is_set == TRUE)
	chanset_ou->chan_set[i].data_ready |= DATA_READY_OU;

  /* Handle the timeout. */
  if (uap->tv) {
    error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
		   sizeof (atv));
    if (error)
      goto done;
    if (itimerfix(&atv)) {
      error = EINVAL;
      goto done;
    }

    /* We set the processor at clock level because we insert a timeout
       to handle the timeval argument of hsl_select() (and thus we must
       avoid the obvious race condition). */
    s = splclock();
    timevaladd(&atv, (struct timeval *)&time);
    timo = hzto(&atv);
    /*
     * Avoid inadvertently sleeping forever.
     */
    if (timo == 0)
      timo = 1;
    splx(s);
  } else
    timo = 0;
 retry:
  /* Remember nselcoll. */
  ncoll = nselcoll;
  p->p_flag |= P_SELECT;

  /* Scan the file descriptors. */
  error = selscan(p, ibits, obits, uap->nd, retval);
  if (error || *retval)
    goto done;

  /* Scan the channels. */
  error = chanscan(p, chanset_in, chanset_ou, chanset_ex, retval);
  if (error || *retval)
    goto done;

  /* Note that before this line, we are not at splhigh processor level,
     then data may have been catched since the last selscan -> we use
     flag P_SELECT and the global variable nselcoll to handle this condition. */
  s = splhigh();
  /* this should be timercmp(&time, &atv, >=) */
  if (uap->tv && (time.tv_sec > atv.tv_sec ||
		  (time.tv_sec == atv.tv_sec && time.tv_usec >= atv.tv_usec))) {
    splx(s);
    goto done;
  }

  /* At selwakeup() time, either nselcoll is incremented (in case of collision,
     because we wake-up every processes) or P_SELECT is unset for a particular
     process. */
  if ((p->p_flag & P_SELECT) == 0 || nselcoll != ncoll) {
    splx(s);
    goto retry;
  }
  p->p_flag &= ~P_SELECT;
  error = tsleep((caddr_t)&selwait, PSOCK | PCATCH, "select", timo);
  splx(s);
  if (error == 0)
    goto retry;
 done:
  p->p_flag &= ~P_SELECT;
  /* select is not restarted after signals... */
  if (error == ERESTART)
    error = EINTR;
  if (error == EWOULDBLOCK)
    error = 0;
#define	putbits(name, x) \
	if (uap->name && \
	    (error2 = copyout((caddr_t)obits[x], (caddr_t)uap->name, ni))) \
		error = error2;
  if (error == 0) {
    int error2;

    putbits(in, 0);
    putbits(ou, 1);
    putbits(ex, 2);
#undef putbits
  }

  /* Update the channel bitmaps and copy them into the user space. */
#define PUT_CHAN_BITS(type, TYPE) \
  if (chanset_##type != NULL && error == 0) { \
    max_idx = -1; \
    for (i = 0; i <= chanset_##type->max_index; i++) \
      if (chanset_##type->chan_set[i].is_set == TRUE && \
	  !(chanset_##type->chan_set[i].data_ready & DATA_READY_##TYPE)) \
	chanset_##type->chan_set[i].is_set = FALSE; \
      else max_idx = i; \
    chanset_##type->max_index = max_idx; \
    error = copyout(chanset_##type, uap->mpcchanset_##type, sizeof(int) + \
                    sizeof(mpc_chan_set_t) * (max_idx + 1)); /* It runs correctly even if max_idx == -1 */ \
  }
  PUT_CHAN_BITS(in, IN);
  PUT_CHAN_BITS(ou, OU);
  PUT_CHAN_BITS(ex, EX);
#undef PUT_CHAN_BITS

  if (chanset_in) free(chanset_in, M_SELECT);
  if (chanset_ou) free(chanset_ou, M_SELECT);
  if (chanset_ex) free(chanset_ex, M_SELECT);

  return error;
}


static int
selscan(p, ibits, obits, nfd, retval)
	struct proc *p;
	fd_mask **ibits, **obits;
	int nfd, *retval;
{
  register struct filedesc *fdp = p->p_fd;
  register int msk, i, j, fd;
  register fd_mask bits;
  struct file *fp;
  int n = 0;
  static int flag[3] = { FREAD, FWRITE, 0 };

  for (msk = 0; msk < 3; msk++) {
    for (i = 0; i < nfd; i += NFDBITS) {
      bits = ibits[msk][i/NFDBITS];
      while ((j = ffs(bits)) && (fd = i + --j) < nfd) {
	bits &= ~(1 << j);
	fp = fdp->fd_ofiles[fd];
	if (fp == NULL)
	  return EBADF;
	if ((*fp->f_ops->fo_select)(fp, flag[msk], p)) {
	  obits[msk][(fd)/NFDBITS] |= 
	    (1 << ((fd) % NFDBITS));
	  n++;
	}
      }
    }
  }
  *retval = n;
  return 0;
}

static int
chanscan(p, chanset_in, chanset_ou, chanset_ex, retval)
         struct proc *p;
	 mpc_chan_set *chanset_in;
	 mpc_chan_set *chanset_ou;
	 mpc_chan_set *chanset_ex;
	 int *retval;
{
  int i, s;
  int n = 0;

  /* Same reason as splclock() in ptc_select() (/sys/kern/tty_pty.c): protection
     of channel_data_ready[][] from an update at interrupt time. */
  s = splhigh();

#define CHANSCAN(type, TYPE) \
  if (chanset_##type) \
    for (i = 0; i <= chanset_##type->max_index; i++) \
      if (chanset_##type->chan_set[i].is_set == TRUE) { \
        if (chanset_##type->chan_set[i].dest >= MAX_NODES) \
          return EBADF; \
        if (chanset_##type->chan_set[i].channel >= CHANNEL_RANGE) \
          return EBADF; \
        if (channel_data_ready[NODE2NODEINDEX(chanset_##type->chan_set[i].dest, my_node)] \
                              [chanset_##type->chan_set[i].channel] & DATA_READY_##TYPE) { \
          chanset_##type->chan_set[i].data_ready |= DATA_READY_##TYPE; \
	  n++; \
        } else \
	  selrecord(p, &channel_select_info_##type[NODE2NODEINDEX(chanset_##type->chan_set[i].dest, my_node)] \
                                                  [chanset_##type->chan_set[i].channel]); \
      }
  CHANSCAN(in, IN);
  CHANSCAN(ou, OU);
  CHANSCAN(ex, EX);
#undef CHANSCAN

  splx(s);
  *retval = n;
  return 0;
}

#else

static int selscan __P((struct proc *, fd_mask **, fd_mask **, int));
static int chanscan __P((struct proc *,
			 mpc_chan_set *, mpc_chan_set *, mpc_chan_set *));

static MALLOC_DEFINE(M_SELECT, "select", "select() buffer");

int
hsl_select(p, uap, retval)
	register struct proc *p;
	register libmpc_select_t *uap;
	int *retval;
{
	/*
	 * The magic 2048 here is chosen to be just enough for FD_SETSIZE
	 * infds with the new FD_SETSIZE of 1024, and more than enough for
	 * FD_SETSIZE infds, outfds and exceptfds with the old FD_SETSIZE
	 * of 256.
	 */
	fd_mask s_selbits[howmany(2048, NFDBITS)];
	fd_mask *ibits[3], *obits[3], *selbits, *sbp;
	struct timeval atv, rtv, ttv;
	int s, ncoll, error, timo;
	u_int nbufbytes, ncpbytes, nfdbits;

	int max_idx, i;
	mpc_chan_set *chanset_in = NULL;
	mpc_chan_set *chanset_ou = NULL;
	mpc_chan_set *chanset_ex = NULL;

	if (uap->nd < 0)
		return (EINVAL);

	if ((uap->mpcchanset_in != NULL && (uap->mpcchanset_in_max_index < 0 ||
					    uap->mpcchanset_in_max_index >= MAX_CHAN_PROC)) ||
	    (uap->mpcchanset_ou != NULL && (uap->mpcchanset_ou_max_index < 0 ||
					    uap->mpcchanset_ou_max_index >= MAX_CHAN_PROC)) ||
	    (uap->mpcchanset_ex != NULL && (uap->mpcchanset_ex_max_index < 0 ||
					    uap->mpcchanset_ex_max_index >= MAX_CHAN_PROC)))
	  return EINVAL;

	if (uap->nd > p->p_fd->fd_nfiles)
		uap->nd = p->p_fd->fd_nfiles;   /* forgiving; slightly wrong */

	/*
	 * Allocate just enough bits for the non-null fd_sets.  Use the
	 * preallocated auto buffer if possible.
	 */
	nfdbits = roundup(uap->nd, NFDBITS);
	ncpbytes = nfdbits / NBBY;
	nbufbytes = 0;
	if (uap->in != NULL)
		nbufbytes += 2 * ncpbytes;
	if (uap->ou != NULL)
		nbufbytes += 2 * ncpbytes;
	if (uap->ex != NULL)
		nbufbytes += 2 * ncpbytes;
	if (nbufbytes <= sizeof s_selbits)
		selbits = &s_selbits[0];
	else
		selbits = malloc(nbufbytes, M_SELECT, M_WAITOK);

	/*
	 * Assign pointers into the bit buffers and fetch the input bits.
	 * Put the output buffers together so that they can be bzeroed
	 * together.
	 */
	sbp = selbits;
#define	getbits(name, x) \
	do {								\
		if (uap->name == NULL)					\
			ibits[x] = NULL;				\
		else {							\
			ibits[x] = sbp + nbufbytes / 2 / sizeof *sbp;	\
			obits[x] = sbp;					\
			sbp += ncpbytes / sizeof *sbp;			\
			error = copyin(uap->name, ibits[x], ncpbytes);	\
			if (error != 0)					\
				goto done;				\
		}							\
	} while (0)
	getbits(in, 0);
	getbits(ou, 1);
	getbits(ex, 2);
#undef	getbits
	if (nbufbytes != 0)
		bzero(selbits, nbufbytes / 2);

#define GET_CHAN_BITS(TYPE) \
  if (uap->mpcchanset_##TYPE != NULL) { \
    chanset_##TYPE = (mpc_chan_set *) malloc(sizeof(int) + \
                     sizeof(mpc_chan_set_t) * (uap->mpcchanset_##TYPE##_max_index + 1), \
                     M_SELECT, M_WAITOK); \
    error = copyin((caddr_t) uap->mpcchanset_##TYPE, chanset_##TYPE, sizeof(int) + \
                   sizeof(mpc_chan_set_t) * (uap->mpcchanset_##TYPE##_max_index + 1)); \
    if (error) goto done; \
    /* Applying basic security rules: the user could have given a different max_index in mpcchanset_in_max_index than in the mpc_chan_set structure. */ \
    if (chanset_##TYPE->max_index != uap->mpcchanset_##TYPE##_max_index) \
        log(LOG_ERR, "hsl_select(): invalid index\n"); \
    chanset_##TYPE->max_index = uap->mpcchanset_##TYPE##_max_index; \
    for (i = 0; i <= chanset_##TYPE->max_index; i++) \
        chanset_##TYPE->chan_set[i].data_ready = 0; \
  }
  GET_CHAN_BITS(in);
  GET_CHAN_BITS(ou);
  GET_CHAN_BITS(ex);
#undef GET_CHAN_BITS

  /* SHOULD BE CHANGED : we do it like if write was always possible on any channel
     (it is easier to code, and I hope it to be a not so bad heuristic...). */
  if (chanset_ou != NULL)
    for (i = 0; i <= chanset_ou->max_index; i++)
      if (chanset_ou->chan_set[i].is_set == TRUE)
	chanset_ou->chan_set[i].data_ready |= DATA_READY_OU;

  /* Handle the timeout. */
	if (uap->tv) {
		error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (atv));
		if (error)
			goto done;
		if (itimerfix(&atv)) {
			error = EINVAL;
			goto done;
		}
		getmicrouptime(&rtv);
		timevaladd(&atv, &rtv);
	} else
		atv.tv_sec = 0;
	timo = 0;
retry:
	/* Remember nselcoll. */
	ncoll = nselcoll;
	p->p_flag |= P_SELECT;

	/* Scan the file descriptors. */
	error = selscan(p, ibits, obits, uap->nd);
	if (error || p->p_retval[0])
		goto done;

	/* Scan the channels. */
	error = chanscan(p, chanset_in, chanset_ou, chanset_ex);
	if (error || p->p_retval[0])
	  goto done;

	if (atv.tv_sec) {
		getmicrouptime(&rtv);
		if (timevalcmp(&rtv, &atv, >=)) 
			goto done;
		ttv = atv;
		timevalsub(&ttv, &rtv);
		timo = ttv.tv_sec > 24 * 60 * 60 ?
		    24 * 60 * 60 * hz : tvtohz(&ttv);
	}
	s = splhigh();
	if ((p->p_flag & P_SELECT) == 0 || nselcoll != ncoll) {
		splx(s);
		goto retry;
	}
	p->p_flag &= ~P_SELECT;
	error = tsleep((caddr_t)&selwait, PSOCK | PCATCH, "select", timo);
	splx(s);
	if (error == 0)
		goto retry;
done:
	p->p_flag &= ~P_SELECT;
	/* select is not restarted after signals... */
	if (error == ERESTART)
		error = EINTR;
	if (error == EWOULDBLOCK)
		error = 0;
#define	putbits(name, x) \
	if (uap->name && (error2 = copyout(obits[x], uap->name, ncpbytes))) \
		error = error2;
	if (error == 0) {
		int error2;

		putbits(in, 0);
		putbits(ou, 1);
		putbits(ex, 2);
#undef putbits
	}

  /* Update the channel bitmaps and copy them into the user space. */
#define PUT_CHAN_BITS(type, TYPE) \
  if (chanset_##type != NULL && error == 0) { \
    max_idx = -1; \
    for (i = 0; i <= chanset_##type->max_index; i++) \
      if (chanset_##type->chan_set[i].is_set == TRUE && \
	  !(chanset_##type->chan_set[i].data_ready & DATA_READY_##TYPE)) \
	chanset_##type->chan_set[i].is_set = FALSE; \
      else max_idx = i; \
    chanset_##type->max_index = max_idx; \
    error = copyout(chanset_##type, uap->mpcchanset_##type, sizeof(int) + \
                    sizeof(mpc_chan_set_t) * (max_idx + 1)); /* It runs correctly even if max_idx == -1 */ \
  }
  PUT_CHAN_BITS(in, IN);
  PUT_CHAN_BITS(ou, OU);
  PUT_CHAN_BITS(ex, EX);
#undef PUT_CHAN_BITS

  if (chanset_in) free(chanset_in, M_SELECT);
  if (chanset_ou) free(chanset_ou, M_SELECT);
  if (chanset_ex) free(chanset_ex, M_SELECT);

	if (selbits != &s_selbits[0])
		free(selbits, M_SELECT);
	return (error);
}

static int
selscan(p, ibits, obits, nfd)
	struct proc *p;
	fd_mask **ibits, **obits;
	int nfd;
{
	register struct filedesc *fdp = p->p_fd;
	register int msk, i, j, fd;
	register fd_mask bits;
	struct file *fp;
	int n = 0;
	/* Note: backend also returns POLLHUP/POLLERR if appropriate. */
	static int flag[3] = { POLLRDNORM, POLLWRNORM, POLLRDBAND };

	for (msk = 0; msk < 3; msk++) {
		if (ibits[msk] == NULL)
			continue;
		for (i = 0; i < nfd; i += NFDBITS) {
			bits = ibits[msk][i/NFDBITS];
			while ((j = ffs(bits)) && (fd = i + --j) < nfd) {
				bits &= ~(1 << j);
				fp = fdp->fd_ofiles[fd];
				if (fp == NULL)
					return (EBADF);
				if ((*fp->f_ops->fo_poll)(fp, flag[msk],
				    fp->f_cred, p)) {
					obits[msk][(fd)/NFDBITS] |=
						(1 << ((fd) % NFDBITS));
					n++;
				}
			}
		}
	}
	p->p_retval[0] = n;
	return (0);
}


static int
chanscan(p, chanset_in, chanset_ou, chanset_ex)
         struct proc *p;
	 mpc_chan_set *chanset_in;
	 mpc_chan_set *chanset_ou;
	 mpc_chan_set *chanset_ex;
{
  int i, s;
  int n = 0;

  /* Same reason as splclock() in ptc_select() (/sys/kern/tty_pty.c): protection
     of channel_data_ready[][] from an update at interrupt time. */
  /* XXX - SHOULD VERIFY IF IT WORKS ON SMP TOO ... */
  s = splhigh();

#define CHANSCAN(type, TYPE) \
  if (chanset_##type) \
    for (i = 0; i <= chanset_##type->max_index; i++) \
      if (chanset_##type->chan_set[i].is_set == TRUE) { \
        if (chanset_##type->chan_set[i].dest >= MAX_NODES) \
          return EBADF; \
        if (chanset_##type->chan_set[i].channel >= CHANNEL_RANGE) \
          return EBADF; \
        if (channel_data_ready[NODE2NODEINDEX(chanset_##type->chan_set[i].dest, my_node)] \
                              [chanset_##type->chan_set[i].channel] & DATA_READY_##TYPE) { \
          chanset_##type->chan_set[i].data_ready |= DATA_READY_##TYPE; \
	  n++; \
        } else \
	  selrecord(p, &channel_select_info_##type[NODE2NODEINDEX(chanset_##type->chan_set[i].dest, my_node)] \
                                                  [chanset_##type->chan_set[i].channel]); \
      }
  CHANSCAN(in, IN);
  CHANSCAN(ou, OU);
  CHANSCAN(ex, EX);
#undef CHANSCAN

  splx(s);
  p->p_retval[0] = n;
  return 0;
}

#endif


