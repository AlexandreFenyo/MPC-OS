
/* $Id: ddslrpp.c,v 1.5 2000/02/08 18:54:31 alex Exp $ */

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "driver.h"
#include "put.h"
#include "ddslrpp.h"
#include "ddslrpv.h"
#include "mdcp.h"
#include "data.h"
#include "cmem.h"
#include "flowcp.h"

#include <sys/systm.h>
#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <sys/syslog.h>

#ifndef DEBUG_MAJOR_EVENTS
/* #define DEBUG_MAJOR_EVENTS */
#endif

#define REMOTETABLE2PHYS(table, node) \
((table ## _entry_t *) ((((void *) (*table)[NODE2NODEINDEX(my_node, node)]) - \
(void *) contig_space) + contig_space_map[node]))
#define LOCALDATA2PHYS(ptr) ((void *) (ptr) - (void *) contig_space + \
			     (void *) contig_space_phys)

/* Callback functions called at close time, to clean-up and wake-up
   sleeping processes */
typedef struct _last_callback {
  boolean_t (*fct) __P((caddr_t));  
  caddr_t param;
} last_callback_t;

/* NOTE that the last callback function may be called many times */
static last_callback_t last_callback[MAX_NODES - NOLOOPBACK]
                                    [CHANNEL_RANGE];

/* mapping from application class name to uid for security management */
uid_t classname2uid[MAX_USER_APPCLASS];

/* Channel rights */
appclassname_t channel_classname[MAX_NODES - NOLOOPBACK]
                                [CHANNEL_RANGE];

/* Protocol used to open a channel */
int channel_protocol[MAX_NODES - NOLOOPBACK]
                    [CHANNEL_RANGE];

/* Data structures to handle the hsl_select() operation */
struct selinfo channel_select_info_in[MAX_NODES - NOLOOPBACK]
                                     [CHANNEL_RANGE];
struct selinfo channel_select_info_ou[MAX_NODES - NOLOOPBACK]
                                     [CHANNEL_RANGE];
struct selinfo channel_select_info_ex[MAX_NODES - NOLOOPBACK]
                                     [CHANNEL_RANGE];
/* channel_data_ready contains a binary combination
   of DATA_READY_IN, DATA_READY_OU and DATA_READY_EX. */
int channel_data_ready[MAX_NODES - NOLOOPBACK]
                      [CHANNEL_RANGE];

/* binary map of active nodes */
static nodes_tab_t nodes_tab;

 /*
  * contig_space_map[] points to remote letter-boxes.
  * contig_space_map[] is not indexed on MAX_NODES - NOLOOPBACK because
  * when we need to fill it, my_node may not be yet initialized,
  * so the NODE2NODEINDEX macro can not be used.
  */
caddr_t contig_space_map[MAX_NODES];

/* binary map of allocated mi */
u_long bitmap_mi[MAX_NODES - NOLOOPBACK]
                [(MI_ID_RANGE + 31) / 32];

/* description of data relative to pending calls to send() */
pending_send_entry_t       pending_send[PENDING_SEND_SIZE];

/* Channel state */
channel_state_t channel_state[MAX_NODES - NOLOOPBACK]
                             [CHANNEL_RANGE];

/* Wired down data for channel shutdown */
u_long wired_trash_data = 0;

/* number of messages sent on a channel */
send_seq_entry_t           send_seq[MAX_NODES - NOLOOPBACK]
                                   [CHANNEL_RANGE];

/* maximum sequence number we noticed in a RECEIVE message */
seq_t                      can_send_seq[MAX_NODES - NOLOOPBACK]
                                       [CHANNEL_RANGE];

/* area used to maintain the localization of local data we need to send */
send_phys_addr_entry_t     send_phys_addr[SEND_PHYS_ADDR_SIZE];

/* letter-boxes used by distant receiver nodes, containing general
   informations about a receive() procedure */
/* this table is contiguous in physical memory */
received_receive_entry_t (*received_receive)[MAX_NODES - NOLOOPBACK]
                                            [RECEIVED_RECEIVE_SIZE];

/* Map maintaining the free and used entries in the letter-boxes.
   A difference with the letter-boxes is that this map is fully maintained
   by the local host (i.e. the sender). */
recv_valid_entry_t         recv_valid[MAX_NODES - NOLOOPBACK]
                                     [RECEIVED_RECEIVE_SIZE];

/* letter-boxes containing the description of distant areas in receivers
   ready to receive data */
/* this table is contiguous in physical memory */
recv_phys_addr_entry_t   (*recv_phys_addr)[MAX_NODES - NOLOOPBACK]
                                          [RECV_PHYS_ADDR_SIZE];

/* Table containing informations about pending calls to receive(). */
/* this table is contiguous in physical memory because it
   also contains the local buffers sent via DMA into the remote
    received_receive[][] letter-boxes */
pending_receive_entry_t  (*pending_receive)[PENDING_RECEIVE_SIZE];

/* this table is contiguous in physical memory */
#ifndef _MULTIPLE_CPA
copy_phys_addr_entry_t   (*copy_phys_addr)[COPY_PHYS_ADDR_SIZE];
#endif

/* number of messages received on a channel */
recv_seq_entry_t           recv_seq[MAX_NODES - NOLOOPBACK]
                                   [CHANNEL_RANGE];

/*
 * bitmap_copy_phys_addr[] is used to accelerate and facilitate some work
 * on copy_phys_addr[]. For this purpose, it was not recommended to
 * add a "valid" field in copy_phys_addr[] because it would then be sent
 * via DMA to the letter boxes.
 */
#ifndef _MULTIPLE_CPA
u_long bitmap_copy_phys_addr[(COPY_PHYS_ADDR_SIZE + 31) / 32];
#endif
u_long bitmap_recv_phys_addr[MAX_NODES - NOLOOPBACK]
                            [(RECV_PHYS_ADDR_SIZE + 31) / 32];

#ifndef _MULTIPLE_CPA
int copy_phys_addr_high;
int copy_phys_addr_low;
#endif

/* send_phys_addr_alloc_entries and send_phys_addr_free_ident could
   be the same variable */
int send_phys_addr_alloc_entries;

int                contig_slot = -1;
vm_offset_t        contig_space = 0;
static vm_offset_t contig_space_backup = NULL;
vm_size_t          contig_size;
caddr_t            contig_space_phys;

/* Various conditions for tsleep/wakeup processing */
int bitmap_mi_free_ident[MAX_NODES - NOLOOPBACK];
int send_phys_addr_free_ident;
#ifndef _MULTIPLE_CPA
int copy_phys_addr_free_ident;
#endif
int pending_send_free_ident;
int pending_receive_free_ident;
int bitmap_recv_phys_addr_free_ident[MAX_NODES - NOLOOPBACK];

static int sap_id;
static int minor;
node_t my_node;
boolean_t timeout_in_progress;
#ifdef _SMP_SUPPORT
struct callout_handle timeout_handle;
#endif

#if (PENDING_SEND_SIZE > 16384) || (PENDING_RECEIVE_SIZE > 16384)
#error max delta between sequences too wide
#endif

/* Check if x < y */
boolean_t
SEQ_INF_STRICT(x, y)
               seq_t x;
               seq_t y;
{
  if (x == y) return FALSE;
  if (x > y) return !SEQ_INF_STRICT(y, x);

  /* Here, we can assume that x < y */

  if (x <= MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE) &&
      y >= 0x10000L - MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE)) {
    if (x + 0x10000L - y > 16 + MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE))
      /* Should not happen... */
      log(LOG_ERR, "SLR: delta between sequences too wide (1)\n");

    return FALSE;
  }

  if (y - x > 16 + MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE))
    /* Should not happen... */
    log(LOG_ERR, "SLR: delta between sequences too wide (2)\n");

  return TRUE;
}

/* Check if x > y */
boolean_t
SEQ_SUP_STRICT(x, y)
               seq_t x;
               seq_t y;
{
  if (x == y) return FALSE;

  return SEQ_INF_STRICT(y, x);
}

/* Check if x <= y */
boolean_t
SEQ_INF_EQUAL(x, y)
              seq_t x;
              seq_t y;
{
  if (x == y) return TRUE;

  return SEQ_INF_STRICT(x, y);
}

/* Check if x >= y */
boolean_t
SEQ_SUP_EQUAL(x, y)
              seq_t x;
              seq_t y;
{
  if (x == y) return TRUE;

  return SEQ_INF_STRICT(y, x);
}

/*
 ************************************************************
 * slrpp_channel_check_rights(node, channel, proc) :
 * check that a process can use a specific channel.
 * node    : destination node.
 * channel : channel number.
 * proc    : proc structure of a process.
 *
 * return values : TRUE  : the process has sufficient rights.
 *                 FALSE : not enough rights.
 ************************************************************
 */

boolean_t
slrpp_channel_check_rights(node, channel, proc)
                           node_t node;
			   channel_t channel;
			   struct proc *proc;
{
  appclassname_t cn;

  if (_PNODE2NODE(node) >= MAX_NODES)
    return FALSE;

  if (channel < MIN_REALLOC_CHANNEL)
    return TRUE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  /* The root can do anything */
  if (!proc->p_ucred->cr_uid) return TRUE;

  cn = channel_classname[NODE2NODEINDEX(node, my_node)][channel];
  if (cn < MIN_USER_APPCLASS || cn >= MAX_USER_APPCLASS) return TRUE;

  /* Compare the EFFECTIVE uid to the uid associated with the class name */
  if (classname2uid[(int) cn] != proc->p_ucred->cr_uid) return FALSE;
  return TRUE;
}


/*
 ************************************************************
 * slrpp_set_appclassname(name, uid) : register the uid associated with
 * an application class name.
 * name : class name.
 * uid  : uid.
 ************************************************************
 */

void
slrpp_set_appclassname(name, uid)
                       appclassname_t name;
                       uid_t uid;
{
  if (name >= MAX_USER_APPCLASS) {
    log(LOG_ERR, "slrpp_set_appclassname: out of range\n");
    return;
  }
  classname2uid[(int) name] = uid;
}


/*
 ************************************************************
 * slrpp_reset_channel(node, channel) : reset the sequence numbers
 * on a channel with a node.
 * node    : remote node.
 * channel : channel identifier.
 ************************************************************
 */

void
slrpp_reset_channel(node, channel)
                    channel_t channel;
                    node_t    node;
{
  int s;

  s = splhigh();

  bzero(&send_seq[NODE2NODEINDEX(node, my_node)][channel],
	sizeof(send_seq[NODE2NODEINDEX(node, my_node)][channel]));
  bzero(&recv_seq[NODE2NODEINDEX(node, my_node)][channel],
	sizeof(recv_seq[NODE2NODEINDEX(node, my_node)][channel]));
  can_send_seq[NODE2NODEINDEX(node, my_node)][channel] = 0;

#ifdef _WITH_DDSCP
  send_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].SFCP_prev_node = -1;
  send_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].SFCP_prev_channel = -1;
  send_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].SFCP_next_node = -1;
  send_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].SFCP_next_channel = -1;
  send_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].SFCP_inside_list = FALSE;

  recv_seq_X[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_prev_node = -1;
  recv_seq_X[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_prev_channel = -1;
  recv_seq_X[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_next_node = -1;
  recv_seq_X[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_next_channel = -1;
  recv_seq_X[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_inside_list = FALSE;

  recv_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_prev_node = -1;
  recv_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_prev_channel = -1;
  recv_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_next_node = -1;
  recv_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_next_channel = -1;
  recv_seq_Y[NODE2NODEINDEX(node, my_node)]
            [channel].RFCP_inside_list = FALSE;
#endif

  splx(s);
}


/*
 ************************************************************
 * slr_clear_config() : clear all the tables used in the protocol.
 *
 * we do not call slr_clear_config() inside init_slr() because a
 * client may send its configuration parameters to a server before
 * another client run on this server (SLR_P on this node is not yet
 * initialized but we must be able to accept configuration parameters
 * from other nodes).
 ************************************************************
 */

void
slr_clear_config()
{
  int s;

  TRY("slr_clear_config")

  s = splhigh();

  bzero(&contig_space_map,
	sizeof contig_space_map);
  bzero(&nodes_tab,
	sizeof nodes_tab);

  splx(s);
}


/*
 ************************************************************
 * search_pending_send(mi) : search an entry in pending_send[]
 * that has a corresponding MI.
 * mi : the mi to look for.
 *
 * return values : a pointer to the entry on success,
 *                 NULL if no entry found.
 ************************************************************
 */

#ifndef _WITH_DDSCP
static pending_send_entry_t *
search_pending_send(mi)
                    mi_t mi;
{
  int i;

  TRY("search_pending_send")

  for (i = 0;
       i < PENDING_SEND_SIZE;
       i++)
    if (pending_send[i].valid == LPE &&
	pending_send[i].mi == mi)
      return pending_send + i;

  return NULL;
}
#endif

/*
 ************************************************************
 * search_pending_send_channel(dest, channel, seq) :
 * search an entry in pending_send[] that has corresponding destination,
 * channel identifier and sequence number.
 * dest    : the remote node.
 * channel : the channel to look for.
 * seq     : the sequence number to look for.
 *
 * return values : a pointer to the entry on success,
 *                 NULL if no entry found.
 ************************************************************
 */

static pending_send_entry_t *
search_pending_send_channel(dest, channel, seq)
                    node_t    dest;
                    channel_t channel;
                    seq_t     seq;
{
  int i;

  TRY("search_pending_send_channel")

  for (i = 0;
       i < PENDING_SEND_SIZE;
       i++)
    if (pending_send[i].valid == VALID &&
	pending_send[i].dest == dest &&
	pending_send[i].channel == channel &&
	pending_send[i].seq == seq)
      return pending_send + i;

  return NULL;
}


/*
 ************************************************************
 * search_pending_receive(sender, mi) : search an entry in
 * pending_receive[] that has a corresponding MI.
 * sender : the remote node.
 * mi     : the MI to look for.
 *
 * return values : a pointer to the entry on success,
 *                 NULL if no entry found.
 ************************************************************
 */

static pending_receive_entry_t *
search_pending_receive(sender, mi)
                       node_t sender;
                       mi_t   mi;
{
  int i;

  TRY("search_pending_receive")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "search_pending_receive()\n");
#endif

  for (i = 0;
       i < PENDING_RECEIVE_SIZE;
       i++)
    if ((*pending_receive)[i].valid == VALID &&
	(*pending_receive)[i].sender == sender &&
	(*pending_receive)[i].mi == mi)
      return (*pending_receive) + i;

  return NULL;
}


/*
 ************************************************************
 * search_received_receive(dest, channel, seq) :
 * search an entry in received_receive[][] that has corresponding
 * destination, channel identifier and sequence number.
 * dest    : the remote node.
 * channel : the channel to look for.
 * seq     : the sequence number to look for.
 *
 * return values : a pointer to the entry on success,
 *                 NULL if no entry found.
 ************************************************************
 */

static received_receive_entry_t *
search_received_receive(dest, channel, seq)
                        node_t dest;
                        channel_t channel;
                        seq_t seq;
{
  received_receive_entry_t *rrentry;
  recv_valid_entry_t       *rventry;
  int i;

  TRY("search_received_receive")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "search_received_receive()\n");
#endif

  rrentry = (*received_receive)[NODE2NODEINDEX(dest, my_node)];
  rventry = recv_valid[NODE2NODEINDEX(dest, my_node)];

  for (i = 1;
       i < RECEIVED_RECEIVE_SIZE;
       rrentry++, rventry++, i++) {
    if (rventry->general.valid == INVALID) continue;
    if (rrentry->general.channel == channel &&
	rrentry->general.seq == seq)
      return rrentry;
  }

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "search_received_receive : return NULL\n");
#endif

  return NULL;
}


/*
 ************************************************************
 * try_send(psentry, rrentry) : try to send data.
 * psentry : an entry in pending_send[] describing the local data
 *           ready to be sent.
 * rrentry : an entry in received_receive[][] describing a remote
 *           set of physical memory pages ready to receive data
 *           described in psentry.
 *
 * This function CAN be called from withing an interrupt context :
 * it never sleeps when there is a lack of resources, it only returns
 * with an error code.
 *
 * return values : 0      : success.
 *                 EFAULT : should not occur, unrecoverable error, contact
 *                          the author if it happens.
 *                 EAGAIN : not enough free entries in the LPE.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 ************************************************************
 */

static int
try_send(psentry, rrentry)
         pending_send_entry_t     *psentry;
         received_receive_entry_t *rrentry;
{
  lpe_entry_t lpe_entry;
  send_phys_addr_entry_t *spaentry;
  recv_phys_addr_entry_t *rpaentry, *rpastart;
  int spaoffset, rpaoffset;
  int nb_pages, res;
  size_t size_sent;

  TRY("send")

  /* size_sent is the size of data really sent : this information
     may be usefull for some applications (for instance, using
     a piggy back and size_sent can be very usefull...) */
  size_sent = 0;

  /************************************************************/
  /* Prepare the LPE entry */
  lpe_entry.routing_part = psentry->dest;
  lpe_entry.control = (rrentry->general.mi & ~((1<<MI_RANGE_SLR) - 1)) +
    MAKE_MI(TYPE_SEND, my_node, MI_ID_PART(rrentry->general.mi));

  /************************************************************/
  /* Compute the number of pages we need to insert into the LPE */

  nb_pages = 1;

  spaentry = psentry->pages;
  rpaentry = rpastart =
    (*recv_phys_addr)[NODE2NODEINDEX(psentry->dest, my_node)] +
    rrentry->general.pages;
  spaoffset = rpaoffset = 0;

  for (ever) {
    /* Should not happen */
    if (spaentry < send_phys_addr) {
      TRY2("PB spaentry")
      log(LOG_ERR,
	  "PB spaentry\n");
      return EFAULT;
    }

    lpe_entry.page_length = MIN(spaentry->size - spaoffset,
				rpaentry->size - rpaoffset);

    spaoffset += lpe_entry.page_length;
    rpaoffset += lpe_entry.page_length;

    /* Break if this was the last entry in send_phys_addr[] */
    if (spaoffset == spaentry->size && !spaentry->next)
      break;

    /* The current send_phys_addr[] entry is fully consumed */
    if (spaoffset == spaentry->size)
      spaoffset = 0, spaentry = spaentry->next;

    /* The current receive_phys_addr[][] entry is fully consumed */
    if (rpaoffset == rpaentry->size)
      rpaoffset = 0, rpaentry++;

    /* Break if this was the last entry in receive_phys_addr[][] */
    if (rpaentry - rpastart == rrentry->general.size)
      break;

    nb_pages++;
  }

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "nb_pages = %d\n",
      nb_pages);
#endif

  /************************************************************/
  /* Add entries into the LPE */

  /* Check that there are enough entries in the LPE for this message */
  if (put_get_lpe_free(minor) < nb_pages)
    return EAGAIN;

  /* With DDSCP, psentry->valid don't change here (and is set to VALID) */
#ifndef _WITH_DDSCP
  psentry->valid = LPE;
#endif
  psentry->mi    = rrentry->general.mi;

  spaentry = psentry->pages;
  rpaentry = rpastart =
    (*recv_phys_addr)[NODE2NODEINDEX(psentry->dest, my_node)]
    + rrentry->general.pages;
  spaoffset = rpaoffset = 0;

  /* Invalidate the entry in received_receive[][] */
  recv_valid[NODE2NODEINDEX(psentry->dest, my_node)]
    [rrentry - (*received_receive)[NODE2NODEINDEX(psentry->dest, my_node)]].
    general.valid = INVALID;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "try_send(): loop\n");
#endif

  for (ever) {
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"try_send(): spaentry = 0x%x / spaoffset = 0x%x / rpaentry = 0x%x / rpaoffset = 0x%x\n",
	(int) spaentry,
	spaoffset,
	(int) rpaentry,
	rpaoffset);
#endif

    /* Set the remote and local data addresses */
    lpe_entry.PLSA = spaentry->address + spaoffset;
    lpe_entry.PRSA = rpaentry->address + rpaoffset;

    lpe_entry.page_length = MIN(spaentry->size - spaoffset,
				rpaentry->size - rpaoffset);

    /* Should not happen */
    if (!lpe_entry.page_length)
      log(LOG_ERR,
	  "zero length page !!!\n");

    spaoffset += lpe_entry.page_length;
    rpaoffset += lpe_entry.page_length;

    /* Break if this was the last entry in send_phys_addr[] */
    if (spaoffset == spaentry->size && !spaentry->next)
      break; /* Last Message Page */

    /* The current send_phys_addr[] entry is fully consumed */
    if (spaoffset == spaentry->size)
      spaoffset = 0, spaentry = spaentry->next;

    /* The current receive_phys_addr[][] entry is fully consumed */
    if (rpaoffset == rpaentry->size)
      rpaoffset = 0, rpaentry++;

    /* Break if this was the last entry in receive_phys_addr[][] */
    if (rpaentry - rpastart == rrentry->general.size)
      break; /* Last Message Page */

    /* Compute the size of data we really send */
    size_sent += lpe_entry.page_length;

    /* Should not happen */
    if (!lpe_entry.page_length) {
      TRY2("put_add_entry_1")
      log(LOG_ERR,
	  "put_add_entry_1\n");
    }

    /* Add the entry into the LPE */
/* PRSA null ici ! */
    TRY2("put_add_entry 1")
    res = put_add_entry(minor,
			&lpe_entry);

    if (res) {
      log(LOG_ERR,
	  "put_add_entry() -> %d\n",
	  res);
      goto error;
    }
  }

  /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
     it is required when using PCI-DDC */
#ifndef _WITH_DDSCP
  lpe_entry.control |= LMP_MASK | NOS_MASK | NOR_MASK | LMI_ENABLE_MASK;
#else
  lpe_entry.control |= LMP_MASK |            NOR_MASK | LMI_ENABLE_MASK;
#endif
  size_sent += lpe_entry.page_length;

  /* Should not happen */
  if (!lpe_entry.page_length) {
    TRY2("put_add_entry_2")
      log(LOG_ERR,
	  "put_add_entry_1\n");
    }

  /* Send the Last Message Page */
  TRY2("put_add_entry 2")
  res = put_add_entry(minor,
		      &lpe_entry);
  if (res) {
    log(LOG_ERR,
	"put_add_entry() -> %d\n",
	res);
    goto error;
  }

#ifndef _WITH_DDSCP
  /* Free send_phys_addr[] */
  {
    int size = 0;

    spaentry = psentry->pages;
    do {
      spaentry->valid = INVALID;
      size++;
      send_phys_addr_alloc_entries--;
    } while ((spaentry = spaentry->next));

    /* Profiler */
    /*
    DECR_VAR_MULT(send_phys_addr, TRACE_DECR_SPA_ENTRY, size);
    */
  }

  /* Wake-up processes waiting for space in send_phys_addr[] */
  wakeup(&send_phys_addr_free_ident);
#endif

  /* Save the actual size of data sent to give this information
     as a parameter of the callback function */
  psentry->size_sent = size_sent;

  res = 0;
error:

  /*
   * Unrecoverable error : when this error occurs, the callback function
   * is not called and some entries in tables will stay indefinitely
   * filled.
   */
  if (res)
    log(LOG_ERR,
	"try_send() unrecoverable error\n");

#ifndef _WITH_DDSCP
  psentry->pages = NULL;
  psentry->size  = 0;
#endif

  return res;
}


/*
 ************************************************************
 * try_recv(prentry) : try to receive data.
 * prentry : an entry in pending_receive[] describing the local area
 *           ready to receive data.
 *
 * This function CAN be called from withing an interrupt context :
 * it never sleeps when there is a lack of resources, it only returns
 * with an error code.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EAGAIN : can't add entry because the LPE is full.
 *                 ERANGE : node out of range.
 *                 ENOMEM : can't allocate enough LPE entries.
 ************************************************************
 */

#ifdef _WITH_DDSCP
extern int
try_recv(prentry)
         pending_receive_entry_t *prentry;
{
  int s;
  int res;
  lpe_entry_t lpe_entry;

  s = splhigh();

  /************************************************************/
  /* We will need 2 or 3 free entries in the LPE */
  if (put_get_lpe_free(minor) < 3)
    return ENOMEM;

  lpe_entry.routing_part = prentry->sender;

  /* We probably should add the first MI ?! */
  lpe_entry.control     = prentry->mi | LRM_ENABLE_MASK;
  lpe_entry.page_length = sizeof(received_receive_entry_t);
  lpe_entry.PLSA        = LOCALDATA2PHYS(&prentry->received_receive);
  lpe_entry.PRSA        = (caddr_t) (MI_ID_PART(prentry->mi) +
				     REMOTETABLE2PHYS(received_receive,
						      prentry->sender));

  if (!lpe_entry.page_length) {
    /* Should not happen */
    TRY2("put_add_entry_3")
    log(LOG_ERR,
	"try_recv(): page length error on put_add_entry_3\n");
  }

  /* Initiate a DMA to the remote letter-box that will contain informations
     about the receive call. */
  TRY2("put_add_entry 3")
  res = put_add_entry(minor,
		      &lpe_entry);
  if (res) {
    splx(s);
    return res;
  }

#ifndef _MULTIPLE_CPA /* This should not happen since _MULTIPLE_CPA is implied by
			 _WITH_DDSCP - the only reason to have _MULTIPLE_CPA without
			 DDSCP would be to save space and in case we only have 2 nodes. */

  /************************************************************/
  /* Do we need to send 1 or 2 pages to transmit the description
     of physical buffers ready to receive data ? */
  if (prentry->copy_phys_addr_high < prentry->backup_copy_phys_addr_high &&
      prentry->copy_phys_addr_high) {
    /* 2 pages */

    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
      (COPY_PHYS_ADDR_SIZE - prentry->backup_copy_phys_addr_high);
    lpe_entry.PLSA        = LOCALDATA2PHYS(*copy_phys_addr +
					   prentry->backup_copy_phys_addr_high);
    lpe_entry.PRSA        = (caddr_t) (prentry->dist_pages +
				       REMOTETABLE2PHYS(recv_phys_addr, prentry->sender));

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_4")
      log(LOG_ERR,
	  "try_recv(): page length error on put_add_entry_4\n");
    }

    TRY2("put_add_entry 4")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      /* Should not happen */
      log(LOG_ERR,
	  "try_recv() : unrecoverable error\n");
      splx(s);
      return res;
    }

    /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
       it is required when using PCI-DDC */
    lpe_entry.control    |= LMP_MASK | NOR_MASK | LMI_ENABLE_MASK;
    lpe_entry.PRSA        = lpe_entry.PRSA + lpe_entry.page_length;
    lpe_entry.PLSA        = LOCALDATA2PHYS(copy_phys_addr);
    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
                            prentry->copy_phys_addr_high;

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_5")
      log(LOG_ERR,
	  "try_recv(): page length error on put_add_entry_5\n");
    }

    TRY2("put_add_entry 5")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      log(LOG_ERR,
	  "try_recv() : unrecoverable error\n");
      splx(s);
      return res;
    }

  } else {
    /* 1 page */

    /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
       it is required when using PCI-DDC */
    lpe_entry.control    |= LMP_MASK | NOR_MASK | LMI_ENABLE_MASK;
    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
      ((prentry->copy_phys_addr_high ? prentry->copy_phys_addr_high : COPY_PHYS_ADDR_SIZE)
       - prentry->backup_copy_phys_addr_high);
    lpe_entry.PRSA        = (caddr_t) (prentry->dist_pages +
				       REMOTETABLE2PHYS(recv_phys_addr, prentry->sender));
    lpe_entry.PLSA        = LOCALDATA2PHYS(*copy_phys_addr +
					   prentry->backup_copy_phys_addr_high);

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_6")
      log(LOG_ERR,
	  "try_recv(): page length error on put_add_entry_6\n");
    }

    TRY2("put_add_entry 6")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      log(LOG_ERR,
	  "try_recv() : unrecoverable error\n");
      splx(s);
      return res;
    }
  }

#else

  /* Sending 1 page */
  /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
     it is required when using PCI-DDC */
  lpe_entry.control    |= LMP_MASK | NOR_MASK | LMI_ENABLE_MASK;
  lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) * prentry->size;
  lpe_entry.PRSA        = (caddr_t) (prentry->dist_pages +
				     REMOTETABLE2PHYS(recv_phys_addr, prentry->sender));
  lpe_entry.PLSA        = LOCALDATA2PHYS(prentry->pages);

  if (!lpe_entry.page_length) {
    /* Should not happen */
    TRY2("put_add_entry_7")
      log(LOG_ERR,
	  "try_recv(): page length error on put_add_entry_7\n");
  }

  TRY2("put_add_entry 7")
  res = put_add_entry(minor,
		      &lpe_entry);
  if (res) {
    log(LOG_ERR,
	"try_recv() : unrecoverable error\n");
    splx(s);
    return res;
  }

#endif

  splx(s);
  return 0; 
}
#endif


/*
 ************************************************************
 * ddslrp_timeout(param) : trying to send data that couldn't be
 * sent before.
 * param : should be NULL.
 *
 * This function is called regularly by clock interrupts if there
 * is some data that should be sent. If it can not send those data,
 * it ask the kernel to be called later to do the job.
 ************************************************************
 */

void
ddslrp_timeout(param)
               void *param;
{
#ifndef _WITH_DDSCP
  int i, res;
#endif
  int s;
#ifdef _WITH_DDSCP
  boolean_t ret;
#endif

#ifdef MPC_STATS
  driver_stats.calls_timeout++;
#endif

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "ddslrp_timeout()\n");
#endif

  TRY("ddslrp_timeout")

  /* 
   * Be aware that calling splhigh() here doesn't give us any absolute
   * warranty that there will be no race condition implied by the
   * treatment done here, because we may have interrupted any SLR
   * section of code not protected by splhigh() itself.
   * We need to call splhigh() here because we are at processor level
   * splclock and we want to be at splnet or higher.
   */

  s = splhigh();

  timeout_in_progress = FALSE;

#ifndef _WITH_DDSCP
  /* Try to send data for each pending_send[] entry that has waiting
     data to be sent */
  /* With DDSCP, we don't want to do the job by ourselves, we want to
     do it at request time */
  for (i = 0; i < PENDING_SEND_SIZE; i++) {
    if (pending_send[i].valid == VALID &&
	pending_send[i].timeout) {
      res = try_send(pending_send + i,
		     pending_send[i].timeout);
      switch (res) {
      case 0:
	break;

      case EAGAIN:
	timeout_in_progress = TRUE;
	break;

      default:
	log(LOG_ERR,
	    "ddslrp_timeout() error\n");
	break;
      }
    }
  }
#endif

#ifdef _WITH_DDSCP
  /* Let FLOWCP do its jobs */
  ret = flowcp_do_jobs(minor);
  if (ret == TRUE)
    timeout_in_progress = TRUE;
#endif

  /* Do we need to do the job again ? */
  if (timeout_in_progress == TRUE) {
    /* Profiler */
#ifdef MPC_STATS
    driver_stats.timeout_timeout++;
#endif

#ifdef _SMP_SUPPORT
    timeout_handle =
#endif
    timeout(ddslrp_timeout,
	    (caddr_t) SLR_P_TIMEOUT_ARG,
	    SLR_P_TIMEOUT_TICKS);
  }

  splx(s);
}


/*
 ************************************************************
 * SLRP_check_jobs(void) : check that SLRP wants a timeout to retry
 * some pending receive().
 * 
 * return value : TRUE  : need a timeout.
 *                FALSE : don't need a timeout.
 ************************************************************
 */

#ifdef _WITH_DDSCP
boolean_t
SLRP_check_jobs()
{
  return FALSE;
}
#endif


/*
 ************************************************************
 * SLRP_do_jobs(void) : try to retransmit pending receive() at the
 * timeout time.
 * 
 * return value : TRUE  : need a timeout.
 *                FALSE : don't need a timeout.
 ************************************************************
 */

#ifdef _WITH_DDSCP
boolean_t
SLRP_do_jobs(minor)
             int minor;
{
  return FALSE;
}
#endif


/*
 ************************************************************
 * interrupt_sent(minor, mi, entry) : treatment of an interrupt for
 * data leaving the LPE.
 * minor : board that generated the interrupt.
 * mi    : the MI associated with this interrupt.
 * entry : the LPE entry associated with this interrupt.
 *
 * This function handles the following two types of messages :
 * SEND and RECEIVE.
 * NOTE THAT THIS INTERRUPT HANDLER IS NEVER CALLED WITH DDSCP.
 ************************************************************
 */

#ifndef _WITH_DDSCP

static void
interrupt_sent(minor, mi, entry)
               int         minor;
               mi_t        mi;
               lpe_entry_t entry;
{
  int i;
#ifndef _MULTIPLE_CPA
  int j, cpastart;
#endif
  pending_send_entry_t *psentry;
  pending_receive_entry_t *prentry;

  TRY("interrupt_sent")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "interrupt_sent()\n");
#endif

  switch (MI_TYPE_PART(mi)) {
  case TYPE_SEND:
    /* data for a message of type SEND have just left the LPE */

    psentry = search_pending_send(put_get_mi_start(minor, sap_id) +
				  MAKE_MI(TYPE_RECV,
					  entry.routing_part,
					  MI_ID_PART(mi)));
    /* Should not happen */
    if (!psentry) {
      log(LOG_ERR,
	  "interrupt_sent() TYPE_SEND error (%d, %d)\n",
	  entry.routing_part,
	  (int) MI_PART(entry.control));
      break;
    }

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
    log(LOG_DEBUG,
	"interrupt sent TYPE_SEND - mi=0x%x id=0x%x channel=0x%x seq=0x%x\n",
	(int) MI_TYPE_PART(mi),
	(int) MI_ID_PART(mi),
	psentry->channel,
	psentry->seq);
#endif

    /* callback */
    if (psentry->fct) psentry->fct(psentry->param,
				   psentry->size_sent);

    /* Free pending_send[] */
    psentry->valid = INVALID;

    /* Profiler */
    /*
    DECR_VAR(pending_send_entry, TRACE_DECR_PS_ENTRY);
    */

    /* wake-up processes */
    wakeup(&pending_send_free_ident);
    break;

  case TYPE_RECV:
    /* data for a message of type RECV have just left the LPE */

    prentry = search_pending_receive(entry.routing_part,
				     MI_PART(entry.control));
    if (!prentry) {
      /* Should not happen */
      log(LOG_ERR,
	  "interrupt_sent() TYPE_RECV error 1\n");
      break;
    }

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
    log(LOG_DEBUG,
	"interrupt sent TYPE_RECV - mi=0x%x id=0x%x channel=0x%x seq=0x%x\n",
	(int) MI_TYPE_PART(mi),
	(int) MI_ID_PART(mi),
	prentry->channel,
	prentry->seq);
#endif

    if (!prentry->pages ||
	!prentry->size) {
      /* Should not happen */
      log(LOG_ERR,
	  "interrupt_sent() TYPE_RECV error 2\n");
      break;
    }

#ifndef _MULTIPLE_CPA

    /* update bitmap_copy_phys_addr[] */
    cpastart = prentry->pages - *copy_phys_addr;
    for (i = cpastart, j = 0;
	 j < prentry->size;
	 i = (i + 1) % COPY_PHYS_ADDR_SIZE, j++)
      (i/32)[bitmap_copy_phys_addr] &= ~(1<<(i % 32));

    /* Profiler */
    /*
    DECR_VAR_MULT(copy_phys_addr, TRACE_DECR_CPA_ENTRY, prentry->size);
    */

    /* update copy_phys_addr_low if the data sent were at the end of 
       the snake */
    if (cpastart == copy_phys_addr_low) {
      while (i != copy_phys_addr_high &&
	     !(bitmap_copy_phys_addr[i / 32] & (1<<(i % 32))))
	i = (i + 1) % COPY_PHYS_ADDR_SIZE;
      copy_phys_addr_low = i;
    }

    prentry->pages = NULL;
    prentry->size  = 0;

    /* wake-up processes */
    wakeup(&copy_phys_addr_free_ident);

#else

    prentry->size  = 0;

#endif

    /* Check that we explicitely reordered the jobs done while in
       interrupt for data sent and interrupt for data received */
    if (prentry->reorder_interrupts == TRUE) {
      /* We need to do the job that would be done during an interrupt
	 for data sent */

      int nindex;

      log(LOG_WARNING, "SLR: interrupt reordered\n");

      /* free mi */
      nindex = NODE2NODEINDEX(prentry->sender, my_node);
      bitmap_mi[nindex][MI_ID_PART(mi) / 32] &= ~(1<<(MI_ID_PART(mi) % 32));

      /* free remote letter-boxes */
      for (i = prentry->dist_pages;
	   i < prentry->dist_pages + prentry->dist_size;
	   i++)
	bitmap_recv_phys_addr[nindex][i / 32] &= ~(1<<(i % 32));

      /* free pending_receive[] */
      prentry->valid = INVALID;
      /* Profiler */
      /*
      DECR_VAR(pending_receive_entry, TRACE_DECR_PR_ENTRY);
      */

      /* callback */
      if (prentry->fct) prentry->fct(prentry->param);

      /* wake-up sleeping processes */
      wakeup(bitmap_mi_free_ident + nindex);
      wakeup(bitmap_recv_phys_addr_free_ident + nindex);
      wakeup(&pending_receive_free_ident);
    }

    break;
  }
}

#endif


/*
 ************************************************************
 * interrupt_received(minor, mi) : treatment of an interrupt for
 * data just arrived from the board.
 * minor : board that generated the interrupt.
 * mi    : the MI associated with this interrupt (+ control flags).
 *
 * This function handles the following two types of messages :
 * SEND and RECEIVE.
 ************************************************************
 */

static void
interrupt_received(minor, mi, data1, data2)
                   int    minor;
                   mi_t   mi;
		   u_long data1;
		   u_long data2;
{
#ifdef _WITH_DDSCP
#ifndef _MULTIPLE_CPA
  int cpastart, j;
#endif
#endif
  int nindex, i, res;
  pending_receive_entry_t *prentry;
  received_receive_entry_t *rrentry;
  pending_send_entry_t *psentry;

  TRY("interrupt_received")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "interrupt_received() / mi = 0x%x\n",
      (int) mi);
#endif

  mi = MI_PART(mi);

  switch (MI_TYPE_PART(mi)) {
  case TYPE_SEND:
    /* data for a message of type SEND have just arrived */

    /* IMPORTANT PATCH NEEDED : in case of DDSCP, we should ignore the interrupt if the entry is in phase of MICP... */

    prentry = search_pending_receive(MI_NODE_PART(mi),
				     put_get_mi_start(minor, sap_id) +
				     MAKE_MI(TYPE_RECV,
					     my_node,
					     MI_ID_PART(mi)));
    if (!prentry) {
      /* Should not happen */
      log(LOG_ERR,
	  "interrupt_received() TYPE_SEND error 1\n");
      break;
    }

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
    log(LOG_DEBUG,
	"interrupt received TYPE_SEND - mi=0x%x id=0x%x channel=0x%x seq=0x%x\n",
	(int) mi,
	(int) MI_ID_PART(mi),
	prentry->channel,
	prentry->seq);
#endif

#ifndef _WITH_DDSCP
    if (prentry->pages || prentry->size) {
      /*
       * Entering this block means we got the second interrupt (data
       * received) of the receiver communication process before the
       * first one (data sent) !
       * This may happen, and we observed it on the emulator after
       * about 10 hours of emulation.
       * To avoid this race condition, we reorder interrupts handling
       * by delaying the job that would normally be done now, until
       * the end of the other interrupt.
       */

      log(LOG_WARNING,
	  "SLR: incorrect order of IT (corrected)\n");

      log(LOG_DEBUG,
	  "interrupt received TYPE_SEND - mi=0x%x id=0x%x channel=0x%x seq=0x%x\n",
	  (int) mi,
	  (int) MI_ID_PART(mi),
	  prentry->channel,
	  prentry->seq);

      prentry->reorder_interrupts = TRUE;

      /* Exit this handler right know */
      break;
    }
#endif

    /* free mi */
    nindex = NODE2NODEINDEX(prentry->sender, my_node);
    bitmap_mi[nindex][MI_ID_PART(mi) / 32] &= ~(1<<(MI_ID_PART(mi) % 32));

    /* free remote letter-boxes */
    for (i = prentry->dist_pages;
	 i < prentry->dist_pages + prentry->dist_size;
	 i++)
      bitmap_recv_phys_addr[nindex][i / 32] &= ~(1<<(i % 32));

    /* free pending_receive[] */
    prentry->valid = INVALID;
#ifdef _WITH_DDSCP
    unregister_job();
#endif
    /* Profiler */
    /*
    DECR_VAR(pending_receive_entry, TRACE_DECR_PR_ENTRY);
    */

#ifdef _WITH_DDSCP

#ifndef _MULTIPLE_CPA
    if (!prentry->pages ||
	!prentry->size) {
      /* Should not happen */
      log(LOG_ERR,
	  "interrupt_received() TYPE_SEND error\n");
      break;
    }
#else
    if (!prentry->size) {
      /* Should not happen */
      log(LOG_ERR,
	  "interrupt_received() TYPE_SEND error\n");
      break;
    }
#endif

#ifndef _MULTIPLE_CPA
    /* update bitmap_copy_phys_addr[] */
    cpastart = prentry->pages - *copy_phys_addr;
    for (i = cpastart, j = 0;
	 j < prentry->size;
	 i = (i + 1) % COPY_PHYS_ADDR_SIZE, j++)
      (i/32)[bitmap_copy_phys_addr] &= ~(1<<(i % 32));

    /* Profiler */
    /*
    DECR_VAR_MULT(copy_phys_addr, TRACE_DECR_CPA_ENTRY, prentry->size);
    */

    /* update copy_phys_addr_low if the data sent were at the end of 
       the snake */
    if (cpastart == copy_phys_addr_low) {
      while (i != copy_phys_addr_high &&
	     !(bitmap_copy_phys_addr[i / 32] & (1<<(i % 32))))
	i = (i + 1) % COPY_PHYS_ADDR_SIZE;
      copy_phys_addr_low = i;
    }

    prentry->pages = NULL;
#endif
    prentry->size  = 0;

    /* wake-up processes */
#ifndef _MULTIPLE_CPA
    wakeup(&copy_phys_addr_free_ident);
#endif

#endif

    /* callback */
    if (prentry->fct) prentry->fct(prentry->param);

#ifdef _WITH_DDSCP
    /* One receive is completely done, thus update recv_seq_X[][] for RFCP
       to inform the sender */
    update_recv_seq_X(prentry->sender, prentry->channel);

    /* Inform RFCP about the new job */
    RFCP_start_process(minor,
		       prentry->sender,
		       prentry->channel);
#endif

    /* wake-up sleeping processes */
    wakeup(bitmap_mi_free_ident + nindex);
    wakeup(bitmap_recv_phys_addr_free_ident + nindex);
    wakeup(&pending_receive_free_ident);

    break;

  case TYPE_RECV:
    /* data for a message of type RECV have just arrived */

    rrentry = (*received_receive)[NODE2NODEINDEX(MI_NODE_PART(mi), my_node)] +
      MI_ID_PART(mi);

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
    log(LOG_DEBUG,
	"interrupt received TYPE_RECV - mi=0x%x id=0x%x channel=0x%x seq=0x%x\n",
	(int) mi,
	(int) MI_ID_PART(mi),
	rrentry->general.channel,
	rrentry->general.seq);
#endif

    recv_valid[NODE2NODEINDEX(MI_NODE_PART(mi), my_node)]
      [MI_ID_PART(mi)].general.valid = VALID;

    /* Update can_send_seq[][] */
    can_send_seq[NODE2NODEINDEX(MI_NODE_PART(mi), my_node)]
                [rrentry->general.channel] =
      MAX_SEQ(can_send_seq[NODE2NODEINDEX(MI_NODE_PART(mi), my_node)]
                          [rrentry->general.channel],
	      1 + rrentry->general.seq);

    psentry = search_pending_send_channel(rrentry->general.dest,
					  rrentry->general.channel,
					  rrentry->general.seq);
    if (!psentry) break; /* this RECEIVE happened before any SEND,
			    no job to do */

    res = try_send(psentry,
		   rrentry);

    /*
     * Testing timeout() : uncomment the following block and comment the
     * preceding one. This test doesn't work if using DDSCP.
     *
     * {
     *   static int i = 0;
     *   if (i < 5) res = EAGAIN; // 5 timeouts before sending data
     *   else res = try_send(psentry, rrentry);
     *   i++;
     * }
     */

    switch (res) {
    case 0:
      break;

    case EAGAIN:
      /* we cannot do the job now (LPE is full), and we cannot wait (can't
	 wait during an interrupt), so we ask for a timeout */
#ifndef _WITH_DDSCP
      if (timeout_in_progress == FALSE) {
	timeout_in_progress = TRUE;
	psentry->timeout = rrentry;
#ifdef _SMP_SUPPORT
	timeout_handle =
#endif
	timeout(ddslrp_timeout,
		(caddr_t) SLR_P_TIMEOUT_ARG,
		SLR_P_TIMEOUT_TICKS);
      }
#endif
      break;

    default:
      log(LOG_ERR,
	  "interrupt_received() TYPE_RECV error 2\n");
      break;
    }

    break;
  }
}


/*
 ************************************************************
 * slrpp_get_nodes() : get a map of the active nodes on the
 * network.
 *
 * return value : a bitmap of active nodes on the network.
 ************************************************************
 */

nodes_tab_t *
slrpp_get_nodes()
{
  int i;

  for (i = 0;
       i < MAX_NODES;
       i++)
    if (contig_space_map[i] ||
	i == my_node)
      nodes_tab[i / 8] |= 1<<(i % 8);

  return &nodes_tab;
}


/*
 ************************************************************
 * set_config_slr(config) : registration of the remote contig.
 * physical memory allocated for use by this node.
 * config : physical address of the allocated memory.
 *
 * set_config_slr can be called even if SLR_P is not initialized to
 * avoid a dead lock in the boot sequence of MPC.
 *
 * return value : 0 on success.
 ************************************************************
 */

int
set_config_slr(config)
               slr_config_t config;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "set_config_slr() [%d]\n",
      config.node);
#endif

  TRY("set_config_slr")

  contig_space_map[config.node] = config.contig_space;
  return 0;
}


/*
 ************************************************************
 * init_slr(m) : allocation of memory and initialization of data
 * structutes used by the SLR/P protocol.
 * m : board to use with SLR/P.
 *
 * This function initializes SLR/P (DDSCP/P) *AND* SLR/V (DDSCP/V).
 * Note that SLR/P can only driver one board at a time.
 *
 * return values : EBUSY   : SLP/P already initialized.
 *                 ENXIO   : board not initialized or not enough
 *                           structures allocated at compilation
 *                           time to handle this board.
 *                 ENOMEM  : not enough memory or not enough MI left.
 *                 ENOBUFS : board not initialized or not enough
 *                           structures allocated at compilation
 *                           time to handle this board.
 *                 ENOENT  : board not initialized.
 *                 EBUSY   : this SAP already has an allocated set
 *                           of MI.
 *                 E2BIG   : range of MI asked is too big to be
 *                           allocated, it should be reduced or
 *                           a new compilation of the
 *                           PUT interface should be required after
 *                           having increased the constant MI_SIZE.
 *                 EINVAL  : put_attach_mi_range() returned EINVAL.
 *                 ...     : some error values returned by init_flowcp().
 ************************************************************
 */

int
init_slr(m)
         int m;
{
  int res;
  int i, j;

  TRY("init_slr")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "init_slr()\n");
#endif

  if (contig_space) return EBUSY; /* previous use was not clean */

  for (i = 0; i < MAX_USER_APPCLASS; i++)
    classname2uid[i] = -1;

  for (i = 0; i < MAX_NODES - NOLOOPBACK; i++)
    for (j = 0; j < CHANNEL_RANGE; j++) {
      channel_classname[i][j] = ~0UL;
      channel_protocol[i][j] = -1;
      last_callback[i][j].fct = NULL;
      last_callback[i][j].param = NULL;
    }

  bzero(channel_select_info_in, sizeof channel_select_info_in);
  bzero(channel_select_info_ou, sizeof channel_select_info_ou);
  bzero(channel_select_info_ex, sizeof channel_select_info_ex);
  bzero(channel_data_ready, sizeof channel_data_ready);

  for (i = 0; i < MAX_NODES - NOLOOPBACK; i++) {
    for (j = 0; j < MIN_REALLOC_CHANNEL; j++)
      channel_state[i][j] = CHAN_OPEN;
    for (j = MIN_REALLOC_CHANNEL; j < CHANNEL_RANGE; j++)
      channel_state[i][j] = CHAN_CLOSE;
  }

  timeout_in_progress = FALSE;
  minor               = m;
  res                 = put_get_node(m);

  if (res < 0) return ENXIO;
  my_node = res;

  send_phys_addr_alloc_entries = 0;
#ifndef _MULTIPLE_CPA
  copy_phys_addr_low = copy_phys_addr_high = 0;
#endif

  /* compute the size of the physical contig. memory we need to allocate */
  contig_size = sizeof(*received_receive) +
                sizeof(*recv_phys_addr) +
#ifndef _MULTIPLE_CPA
                sizeof(*copy_phys_addr) +
#endif
                sizeof(*pending_receive);

  if (!contig_space_backup) {
    contig_slot  = cmem_getmem(contig_size, "SLR/P");
    if (contig_slot < 0) {
      contig_space = NULL;
      return ENOMEM;
    }

    contig_space = cmem_virtaddr(contig_slot);
    contig_space_phys = (caddr_t) cmem_physaddr(contig_slot);
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "init_slr: contig_size = 0x%x\n",
	contig_size);
    log(LOG_DEBUG,
	"init_slr: contig_space = 0x%x\n",
	contig_space);
    log(LOG_DEBUG,
	"init_slr: contig_space_phys = 0x%x\n",
	(int) contig_space_phys);
#endif
    if (!contig_space)
      return ENOMEM;
    contig_space_backup = contig_space;
  } else {
    contig_space = contig_space_backup;
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"init_slr: REUSE of contig_space_backup\n");
#endif
  }

#ifndef _MULTIPLE_CPA
  received_receive = (void *) contig_space;
  recv_phys_addr   = (void *) received_receive + sizeof(*received_receive);
  copy_phys_addr   = (void *) recv_phys_addr   + sizeof(*recv_phys_addr);
  pending_receive  = (void *) copy_phys_addr   + sizeof(*copy_phys_addr);
#else
  received_receive = (void *) contig_space;
  recv_phys_addr   = (void *) received_receive + sizeof(*received_receive);
  pending_receive  = (void *) recv_phys_addr   + sizeof(*recv_phys_addr);
#endif

  bzero((void *) contig_space,
	contig_size);

#ifdef BZERO
#error BZERO already defined
#endif

#define BZERO(table) bzero((table), sizeof(table))

  /*
   * We don't sweep contig_space_map here because it can already be
   * initialized (slr_clear_config() will do the job)
   */
#ifndef _MULTIPLE_CPA
  BZERO(bitmap_copy_phys_addr);
#endif
  BZERO(bitmap_recv_phys_addr);
  BZERO(bitmap_mi);
  BZERO(pending_send);
  BZERO(send_seq);
  BZERO(can_send_seq);
  BZERO(send_phys_addr);
  BZERO(recv_valid);
  BZERO(recv_seq);

#undef BZERO

  sap_id = put_register_SAP(m,
#ifdef _WITH_DDSCP
			    NULL,
#else
			    interrupt_sent,
#endif
			    interrupt_received);
  if (sap_id < 0) {
    contig_space      = NULL;
    return ENOBUFS;
  }

  res = put_attach_mi_range(m,
			    sap_id,
			    1<<MI_RANGE_SLR);
  if (res) {
    put_unregister_SAP(m,
		       sap_id);
    contig_space      = NULL;
    return res;
  }

  ADD_EVENT(TRACE_INIT_SLR_P);
  ADD_EVENT(TRACE_INIT_SLR_V);

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "init_slr(): OK\n");
#endif

#ifdef _WITH_DDSCP
  /* Initialize RFCP/SFCP/MICP */
  res = init_flowcp(m);
  if (res < 0)
    return res;
#endif

  /* Initialize the upper layers */
#ifdef WITH_STABS_PCIBUS_SET
  slrpv_prot_init();
#endif
  mdcp_init();

  return 0;
}


/*
 ************************************************************
 * end_slr() : close the SLR/P service, but don't free the allocated
 * physical contig. space.
 ************************************************************
 */

void
end_slr()
{
  int s;

  TRY("end_slr")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "end_slr()\n");
#endif

  /* Close the upper layers */
#ifdef WITH_STABS_PCIBUS_SET
  slrpv_prot_end();
#endif
  mdcp_end();

  /* Unregister SLR/P from PUT */
  if (contig_space) {
    put_unregister_SAP(minor,
		       sap_id);
    contig_space      = NULL;

    ADD_EVENT(TRACE_END_SLR_V);
    ADD_EVENT(TRACE_END_SLR_P);
  }

#ifdef _WITH_DDSCP
  /* Close RFCP/SFCP/MICP */
  end_flowcp(minor);
#endif

  s = splhigh();
  if (timeout_in_progress == TRUE) {
    untimeout(ddslrp_timeout,
	      (caddr_t) SLR_P_TIMEOUT_ARG
#ifdef _SMP_SUPPORT
	      , timeout_handle
#endif
	      );
    timeout_in_progress = FALSE;
  }
  splx(s);
}


/*
 ************************************************************
 * close_slr() : definitively close the SLR/P service, the
 * allocated physical contig. space is freed.
 ************************************************************
 */

void
close_slr()
{
  TRY("close_slr")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "close_slr()\n");
#endif

  if (contig_space)
    put_unregister_SAP(minor,
		     sap_id);

  if (contig_slot != -1) {
    cmem_releasemem(contig_slot);
    contig_slot = -1;
  }

  contig_space = NULL;
  contig_space_phys = NULL;
  contig_space_backup = NULL;
}


/*
 ************************************************************
 * slrpp_cansend(dest, channel) :
 * check if there is a pending receive on the other side of
 * the channel.
 * dest    : remote node.
 * channel : channel identifier.
 *
 * return values : TRUE  : one or more pending receive.
 *                 FALSE : no pending receive.
 ************************************************************
 */

boolean_t
slrpp_cansend(dest, channel)
              node_t dest;
	      channel_t channel;
{
#ifndef _WITH_DDSCP

  /* NOTE that we could/SHOULD use the code after the #else attribute,
     in the case of DDSCP, and it should be faster (no loop) */

  int s, i;
  seq_t seq;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return FALSE;

  if (channel >= CHANNEL_RANGE)
    return FALSE;

  s = splhigh();

  seq = send_seq[NODE2NODEINDEX(dest, my_node)][channel].seq;

  for (i = 1;
       i < RECEIVED_RECEIVE_SIZE;
       i++) {
    if (recv_valid[NODE2NODEINDEX(dest, my_node)]
                  [i].general.valid != INVALID &&
	(*received_receive)[NODE2NODEINDEX(dest, my_node)]
                           [i].general.channel == channel &&
	(*received_receive)[NODE2NODEINDEX(dest, my_node)]
	                   [i].general.seq == seq) {
      splx(s);

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "cansend TRUE: seq=%x\n", seq);
#endif

      return TRUE;
    }
  }

#ifdef DEBUG_HSL
  log(LOG_DEBUG, "cansend FALSE: seq=%x\n", seq);
#endif

  splx(s);
  return FALSE;

#else

  int s, ret;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return FALSE;

  if (channel >= CHANNEL_RANGE)
    return FALSE;

  s = splhigh();

  if (SEQ_INF_STRICT(send_seq[NODE2NODEINDEX(dest, my_node)][channel].seq,
		     can_send_seq[NODE2NODEINDEX(dest, my_node)][channel]))
    ret = TRUE;
  else ret = FALSE;

  splx(s);
  return ret;

#endif
}


/*
 ************************************************************
 * slrpp_send(dest, channel, pages, size, fct, param, proc) :
 * try to send data to a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * pages   : description of pages to send.
 * size    : number of pages to send - each page MUST contain less
 *           than 64Kbytes.
 * fct     : callback function called when data have been correctly
 *           sent.
 * param   : parameter provided to the callback function when called.
 * proc    : proc structure of the calling process.
 *
 * If the data cannot be sent, a timeout() is generated and
 * the kernel will call ddslrp_timeout() until this procedure
 * can effectively send the data.
 *
 * return values : 0           : success.
 *                 ERANGE      : channel or dest node out of range.
 *                 ENXIO       : SLR/P not initialized or not enough
 *                               structures allocated at compilation
 *                               time to handle this board.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (in case of local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 EFAULT      : should not occur, unrecoverable error,
 *                               contact the author if it happens.
 *                 E2BIG       : invalid size of a page.
 *                 ENOENT      : this board is not initialized.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

__inline int
slrpp_send(dest, channel, pages, size, fct, param, proc)
           node_t dest;
           channel_t channel;
           pagedescr_t *pages;
           size_t size;
           void (*fct) __P((caddr_t, int));
           caddr_t param;
           struct proc *proc;
{
  return _slrpp_send(dest, channel, pages, size, fct, param, NULL, proc, FALSE);
}

int
_slrpp_send(dest, channel, pages, size, fct, param, psentry_p, proc, bypass)
            node_t dest;
            channel_t channel;
            pagedescr_t *pages;
            size_t size;
            void (*fct) __P((caddr_t, int));
            caddr_t param;
            pending_send_entry_t **psentry_p;
            struct proc *proc;
	    boolean_t bypass;
{
  int res, i, j, s/*, len*/;
  int pending_send_index;
  seq_t seq;
  send_phys_addr_entry_t *last;
  send_phys_addr_entry_t *volatile first;
  received_receive_entry_t *rrentry;

  /*
   * We declare `first' as volatile because when compiling with
   * optimization, gcc warns about it, but should not... We solve the
   * problem with volatile because this warning (uninitialized) occurs
   * only for variables that are candidates for register allocation.
   */

  TRY("slrpp_send")

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
  log(LOG_ERR, "_slrpp_send(dst=%d,chn=%d)\n", dest, channel);
#endif

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (!contig_space) return ENXIO;

  if (!size) {
    log(LOG_ERR,
	"slrpp_send(): size is NULL!\n");
    return EINVAL;
  }

#ifndef WITH_LOOPBACK
  if (dest == my_node)
    return EINVAL;
#endif

  for (i = 0; i < size; i++)
    if (!pages[i].size ||
	pages[i].size >= 0x10000UL)
      return E2BIG;

  s = splhigh();

#ifdef MPC_PERFMON
  __asm__("cli");
  PERFMON_START;
#endif

#ifdef MPC_STATS
  driver_stats.calls_slrpp_send++;
#endif

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  /* Profiler */
  /*
  switch (channel) {
  case 0:
    INCR_VAR(sequence_0_out, TRACE_CHAN_0_OUT);

    len = 0;
    for (i = 0; i < size; i++)
      len += pages[i].size;
    INCR_VAR_MULT(channel_0_out, TRACE_CHAN_0_OUT, len);
    break;

  case 1:
    INCR_VAR(sequence_1_out, TRACE_CHAN_1_OUT);

    len = 0;
    for (i = 0; i < size; i++)
      len += pages[i].size;
    INCR_VAR_MULT(channel_1_out, TRACE_CHAN_1_OUT, len);
    break;

  default:
    break;
  }
  */

  /************************************************************/
  /* Check for resources                                      */
  /************************************************************/

restart: /* after sleeping, we restart here to complete the operation
	    whether not every resources were available at the same time */

  /************************************************************/
  /* find a free entry in pending_send[] */
  for (i = 0;
       i < PENDING_SEND_SIZE;
       i++)
    if (pending_send[i].valid == INVALID ||
	pending_send[i].post_treatment_lock == 1)
      break;

  if (i == PENDING_SEND_SIZE) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&pending_send_free_ident,
		 HSLPRI | PCATCH,
		 "PSENDFREE",
		 NULL);
    if (!res)
      goto restart; /* TRY AGAIN */

#ifdef MPC_PERFMON
    __asm__("sti");
#endif
    splx(s);
    return res;
  }
  pending_send_index = i;

  if (psentry_p != NULL)
    *psentry_p = pending_send + i;

  /************************************************************/
  /* find free entries in send_phys_addr[] */

  if (SEND_PHYS_ADDR_SIZE - send_phys_addr_alloc_entries < size) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&send_phys_addr_free_ident,
		 HSLPRI | PCATCH,
		 "SPHYSFREE",
		 NULL);

    if (!res)
      goto restart; /* TRY AGAIN */

#ifdef MPC_PERFMON
    __asm__("sti");
#endif
    splx(s);
    return res;
  }

  /* Profiler */
  /*
  INCR_VAR_MULT(send_phys_addr, TRACE_INCR_SPA_ENTRY, size);
  */

  /************************************************************/
  /* check that the channel is opened */
  /* NO tsleep() MUST OCCUR AFTER THIS STEP */

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  /************************************************************/
  /* Allocate resources                                       */
  /************************************************************/

  /************************************************************/
  /* remember and increment the sequence index */
  seq = send_seq[NODE2NODEINDEX(dest, my_node)][channel].seq++;

#ifdef DEBUG_HSL
  log(LOG_DEBUG, "SEND seq=%x\n", seq);
#endif

  /************************************************************/
  /* fill in send_phys_addr[] */

  send_phys_addr_alloc_entries += size;

  /* Should not happen ! */
  if (send_phys_addr_alloc_entries > SEND_PHYS_ADDR_SIZE)
    log(LOG_ERR,
	"out of capacity ERROR !\n");

  last = NULL;
  j = 0;
  for (i = 0; j < size; i++)
    if (send_phys_addr[i].valid == INVALID) {
      /* We found a free entry */

      if (!j)
	first = send_phys_addr + i;

      /* Fill in the entry */

      /* Should not happen ! */
      if (send_phys_addr[i].valid == VALID)
	log(LOG_ERR,
	    "send_phys_addr[] CORRUPTED !\n");

      send_phys_addr[i].valid   = VALID;
      send_phys_addr[i].address = pages[j].addr;
      send_phys_addr[i].size    = pages[j].size;

      /* Update the previous entry in the list */
      if (last)
	last->next = send_phys_addr + i;
      last = send_phys_addr + i;

      j++;

      /* Should not happen ! */
      if (j < size && i + 1 >= SEND_PHYS_ADDR_SIZE)
	log(LOG_ERR,
	    "memory CORRUPTED !\n");
    }
  if (last)
    last->next = NULL;

  /************************************************************/
  /* fill in pending_send[] */

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
  {
    size_t nbytes;
    int pagecnt;

    nbytes = 0;
    for (pagecnt = 0; pagecnt < size; pagecnt++)
      nbytes += pages[pagecnt].size;

    log(LOG_WARNING,
	"Inserting a pending_send_entry[%d] : seq=%d nbytes=%d\n",
	pending_send_index,
	seq,
	nbytes);
  }
#endif

  pending_send[pending_send_index].dest    = dest;
  pending_send[pending_send_index].channel = channel;
  pending_send[pending_send_index].seq     = seq;
  pending_send[pending_send_index].pages   = first;
  pending_send[pending_send_index].size    = size;
  pending_send[pending_send_index].proc    = proc;
  pending_send[pending_send_index].fct     = fct;
  pending_send[pending_send_index].param   = param;
  pending_send[pending_send_index].timeout = NULL;
  pending_send[pending_send_index].valid   = VALID;
  /* MI should not been initialized, but for debugging,
     it may be usefull to know if the MI was modified... */
  pending_send[pending_send_index].mi      = ~0;

  pending_send[pending_send_index].post_treatment_lock =
    (psentry_p != NULL) ? 1 : 0;

  bzero(pending_send[pending_send_index].prot_map_ref_tab,
	sizeof(pending_send[pending_send_index].prot_map_ref_tab));

  /* Profiler */
  /*
  INCR_VAR(pending_send_entry, TRACE_INCR_PS_ENTRY);
  */

  /*************************************************************/
  /* search an entry in received_receive[][] matching this one */

  rrentry = search_received_receive(dest,
				    channel,
				    seq);
  if (!rrentry) {
#ifdef _WITH_DDSCP
    /* Inform SFCP that send_seq has been incremented */
    SFCP_start_process(minor,
		       dest,
		       channel);
#endif

#ifdef MPC_PERFMON
    PERFMON_STOP("WITHOUT try_send()");
    __asm__("sti");
#endif

    splx(s);
    return 0;
  }

  /************************************************************/
  /* Try to send data                                         */
  /************************************************************/

#ifdef _WITH_DDSCP

  res = try_send(pending_send + pending_send_index,
		 rrentry);

  if (res && res != EAGAIN)
    /* Should not happen */
    log(LOG_ERR,
	"slrpp_send(): fatal error when trying to send : %d\n",
	res);

  /* Inform SFCP that send_seq has been incremented */
  SFCP_start_process(minor,
		     dest,
		     channel);

#ifdef MPC_PERFMON
  if (!res) PERFMON_STOP("WITH try_send()");
  __asm__("sti");
#endif

  splx(s);

  if (!res || res == EAGAIN)
    return 0;
  else
    return res;

#else

  do {
    res = try_send(pending_send + pending_send_index,
		   rrentry);

    if (res &&
	res != EAGAIN) {
    /* Should not happen */
      log(LOG_ERR,
	  "slrpp_send(): fatal error when trying to send : %d\n",
	  res);
#ifdef MPC_PERFMON
      __asm__("sti");
#endif
      splx(s);
      return res;
    }

    if (!res) {
      /* Data sent, exit from slrpp_send() */

#ifdef MPC_PERFMON
      PERFMON_STOP("WITH try_send()");
      __asm__("sti");
#endif

      splx(s);
      return 0;
    }

    /* Here, res equals EAGAIN */

    /*
     * the following tsleep() cannot be interrupted because if try_send()
     * is not called with success before we return, some tables will have
     * entries permanently used.
     */

    /* Wait for free entries in the LPE */
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&hslfreelpe_ident,
		 HSLPRI,
		 "FREELPE",
		 NULL);

    /* If tsleep returns normally, try again to send data */
  } while (!res);

  /* Here, tsleep() did not return normally */

  log(LOG_ERR,
      "slrpp_send(): fatal error : tsleep returned incorrectly\n");

#ifdef MPC_PERFMON
  __asm__("sti");
#endif
  splx(s);

  /*
   * Note that the following line is commented because tsleep()
   * is not called with PCATCH flag, thus it can not return with
   * ERESTART
   */
  /* if (res == ERESTART) res = EINTR; */

  return res;

#endif
}


/*
 ************************************************************
 * slrpp_recv(dest, channel, pages, size, fct, param, proc) :
 * ask to receive data from a remote node on a specific channel.
 * dest     : remote node.
 * channel  : channel identifier.
 * pages    : description of pages ready to receive data.
 * size     : number of pages ready to receive data - each page MUST
 *            contain less than 64Kbytes.
 * fct      : callback function called when data have been correctly
 *            received.
 * param    : parameter provided to the callback function when called.
 * proc     : proc structure of the calling process.
 *
 * return values : 0           : success.
 *                 ERANGE      : channel or dest node out of range.
 *                 ENXIO       : SLR/P not initialized.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (in case of local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 ENXIO       : not enough structures allocated at
 *                             : compilation time to handle this board.
 *                 E2BIG       : invalid size of a page.
 *                 ENOENT      : this board is not initialized.
 *                 ENOMEM      : size too big.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

__inline int
slrpp_recv(dest, channel, pages, size, fct, param, proc)
           node_t dest;
           channel_t channel;
           pagedescr_t *pages;
           size_t size;
           void (*fct) __P((caddr_t));
           caddr_t param;
           struct proc *proc;
{
  return _slrpp_recv(dest, channel, pages, size, fct, param, NULL, proc, FALSE);
}

int
_slrpp_recv(dest, channel, pages, size, fct, param, prentry_p, proc, bypass)
            node_t dest;
            channel_t channel;
            pagedescr_t *pages;
            size_t size;
            void (*fct) __P((caddr_t));
            caddr_t param;
            pending_receive_entry_t **prentry_p;
            struct proc *proc;
	    boolean_t bypass;
{
#ifndef _WITH_DDSCP
  lpe_entry_t lpe_entry;
#endif
  int i, nindex, free_range, s, res;
  /* int len; */
#ifndef _MULTIPLE_CPA
  int free_entries;
#endif
  int pr;
  mi_t mi_id;
  seq_t seq;
#ifndef _MULTIPLE_CPA
  copy_phys_addr_entry_t *pages_start;
  int backup_copy_phys_addr_high;
#endif

  TRY("slrpp_recv")

#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
  log(LOG_ERR, "_slrpp_recv(dst=%d,chn=%d)\n", dest, channel);
#endif

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (!contig_space)
    return ENXIO;

  if (!size) {
    log(LOG_ERR,
	"slrpp_recv(): size is NULL!\n");
    return EINVAL;
  }

#ifndef WITH_LOOPBACK
  if (dest == my_node)
    return EINVAL;
#endif

  for (i = 0; i < size; i++)
    if (!pages[i].size ||
	pages[i].size >= 0x10000UL)
      return E2BIG;

  s = splhigh();

#ifdef MPC_STATS
  driver_stats.calls_slrpp_recv++;
#endif

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  /* Profiler */
  /*
  switch (channel) {
  case 0:
    INCR_VAR(sequence_0_in, TRACE_CHAN_0_IN);
    break;

  case 1:
    INCR_VAR(sequence_1_in, TRACE_CHAN_1_IN);
    break;

  default:
    break;
  }
  */

  /* Profiler */
  /*
  len = 0;
  for (i = 0; i < size; i++)
    len += pages[i].size;
  INCR_VAR_MULT(channel_0_in, TRACE_CHAN_0_IN, len);
  */

  /************************************************************/
  /* Check for resources                                      */
  /************************************************************/

restart: /* after sleeping, we restart here to complete the operation
	    when not every resources are available at the same time */

  /************************************************************/
  /* We will need 2 or 3 free entries in the LPE */
  if (put_get_lpe_free(minor) < 3) {

    /* Waiting for available space in the LPE */
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&hslfreelpe_ident,
		 HSLPRI | PCATCH,
		 "FREELPE",
		 0);
    if (!res)
      goto restart; /* TRY AGAIN */

    splx(s);

    /*
     * Don't want to retry the system call even if a signal with
     * the SA_RESTART flag interrupted it
     */
    if (res == ERESTART) res = EINTR;

    return res;
  }

  /************************************************************/
  /* find a free entry in pending_receive[] */
  for (i = 0;
       i < PENDING_RECEIVE_SIZE;
       i++)
    if ((*pending_receive)[i].valid == INVALID ||
	(*pending_receive)[i].post_treatment_lock == 1)
      break;

  if (i == PENDING_RECEIVE_SIZE) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&pending_receive_free_ident,
		 HSLPRI | PCATCH,
		 "PRECVFREE",
		 NULL);
    if (!res)
      goto restart; /* TRY AGAIN */

    splx(s);

    /*
     * Don't want to retry the system call even if a signal with
     * the SA_RESTART flag interrupted it
     */
    if (res == ERESTART) res = EINTR;

    return res;
  }
  pr = i;

  if (prentry_p != NULL)
    *prentry_p = (*pending_receive) + i;

  /************************************************************/
  /* find free entries in copy_phys_addr[] */
#ifndef _MULTIPLE_CPA

  free_entries = (copy_phys_addr_high >= copy_phys_addr_low) ?
    COPY_PHYS_ADDR_SIZE + copy_phys_addr_low - copy_phys_addr_high - 1 :
    (copy_phys_addr_low - copy_phys_addr_high - 1);
  if (free_entries < size) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(&copy_phys_addr_free_ident,
		 HSLPRI | PCATCH,
		 "CPHYSFREE",
		 NULL);
    if (!res)
      goto restart; /* TRY AGAIN */

    splx(s);

    /*
     * Don't want to retry the system call even if a signal with
     * the SA_RESTART flag interrupted it
     */
    if (res == ERESTART) res = EINTR;

    return res;
  }

#else

  if (size > INTERNAL_CPA_SIZE) {
    log(LOG_ERR, "Fatal error: internal copy_phys_addr table too small\n");
    splx(s);
    return ENOMEM;
  }

#endif

  /************************************************************/
  /* find a free mi in bitmap_mi[][] */
  nindex = NODE2NODEINDEX(dest,
			  my_node);
  for (mi_id = 1;
       mi_id < MI_ID_RANGE;
       mi_id++)
    if (!(bitmap_mi[nindex][mi_id / 32] & (1<<(mi_id % 32))))
      break;
  if (mi_id == MI_ID_RANGE) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(bitmap_mi_free_ident + nindex,
		 HSLPRI | PCATCH,
		 "MI",
		 NULL);
    if (!res)
      goto restart; /* TRY AGAIN */

    splx(s);

    /*
     * Don't want to retry the system call even if a signal with
     * the SA_RESTART flag interrupted it
     */
    if (res == ERESTART) res = EINTR;

    return res;
  }

  /************************************************************/
  /* find free entries in recv_phys_addr[][] */
  free_range = 0;
  for (i = 0;
       i < RECV_PHYS_ADDR_SIZE;
       i++) {
    if (bitmap_recv_phys_addr[nindex][i / 32] & (1<<(i % 32)))
      free_range = 0;
    else free_range++;
    if (free_range == size)
      break;
  }
  if (i == RECV_PHYS_ADDR_SIZE) {
#ifdef MPC_PERFMON
    perfmon_invalid = 1;
#endif
    res = tsleep(bitmap_recv_phys_addr_free_ident + nindex,
		 HSLPRI | PCATCH,
		 "RPHYSFREE",
		 0);
    if (!res)
      goto restart; /* TRY AGAIN */

    splx(s);

    /*
     * Don't want to retry the system call even if a signal with
     * the SA_RESTART flag interrupted it
     */
    if (res == ERESTART) res = EINTR;

    return res;
  }
  free_range = i - size + 1;

  /************************************************************/
  /* check that the channel is opened */
  /* NO tsleep() MUST OCCUR AFTER THIS STEP */

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (bypass == FALSE &&
      channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  /************************************************************/
  /* Allocate resources                                       */
  /************************************************************/

  /************************************************************/
  /* remember and increment the sequence index */
  seq = recv_seq[NODE2NODEINDEX(dest, my_node)][channel].seq++;

#ifdef DEBUG_HSL
  log(LOG_DEBUG, "RECEIVE seq=%x\n", seq);
#endif

  /************************************************************/
  /* fill in copy_phys_addr[] */
#ifndef _MULTIPLE_CPA

  pages_start = *copy_phys_addr + copy_phys_addr_high;
  backup_copy_phys_addr_high = copy_phys_addr_high;
  for (i = 0;
       i < size;
       i++) {
    bitmap_copy_phys_addr[copy_phys_addr_high / 32] |=
      1<<(copy_phys_addr_high % 32);
    (*copy_phys_addr)[copy_phys_addr_high++].pagedescr = pages[i];
    copy_phys_addr_high %= COPY_PHYS_ADDR_SIZE;
  }

  /* Profiler */
  /*
  INCR_VAR_MULT(copy_phys_addr, TRACE_INCR_CPA_ENTRY, size);
  */

#else

  /* the copy is done later in this function */

#endif

  /************************************************************/
  /* fill in bitmap_mi[][] */
  bitmap_mi[nindex][mi_id / 32] |=  1<<(mi_id % 32);


  /************************************************************/
  /* fill in bitmap_recv_phys_addr[][] */
  for (i = free_range;
       i < free_range + size;
       i++)
    bitmap_recv_phys_addr[nindex][i / 32] |= 1<<(i % 32);

  /************************************************************/
  /* fill in pending_receive[] */
#if defined DEBUG_HSL || defined DEBUG_MAJOR_EVENTS
  {
    size_t nbytes;
    int pagecnt;

    nbytes = 0;
    for (pagecnt = 0; pagecnt < size; pagecnt++)
      nbytes += pages[pagecnt].size;

    log(LOG_WARNING,
	"Inserting a pending_receive_entry[%d] : seq=%d nbytes=%d\n",
	pr,
	seq,
	nbytes);
  }
#endif

#define pnr (*pending_receive)[pr]

  pnr.sender     = dest;
  pnr.mi         = put_get_mi_start(minor, sap_id) +
                   MAKE_MI(TYPE_RECV, my_node, mi_id);
  pnr.channel    = channel;
  pnr.seq        = seq;
#ifndef _MULTIPLE_CPA
  pnr.pages      = pages_start;
#else
  bcopy(pages, pnr.pages, size * sizeof(copy_phys_addr_entry_t));
#endif
  pnr.size       = size;
  pnr.dist_pages = free_range;
  pnr.dist_size  = size;
  pnr.proc       = proc;
  pnr.fct        = fct;
  pnr.param      = param;


  pnr.post_treatment_lock = (prentry_p != NULL) ? 1 : 0;
  pnr.reorder_interrupts = FALSE;

  bzero(pnr.prot_map_ref_tab, sizeof(pnr.prot_map_ref_tab));

#ifdef _WITH_DDSCP
  if (SEQ_INF_EQUAL(send_seq_Y[NODE2NODEINDEX(dest, my_node)][channel].seq, seq))
    /* There is no send() for this receive() */
    pnr.stage = STAGE_START;
  else {
    /* There already is a send() for this receive() */
    pnr.stage    = STAGE_MICP;
    pnr.micp_seq = 0;
    register_job();
  }
#endif

  pnr.valid      = VALID;

  /* Profiler */
  /* INCR_VAR(pending_receive_entry, TRACE_INCR_PR_ENTRY); */

  /* fill in pending_receive[].received_receive */
#define prrr (*pending_receive)[pr].received_receive.general

  prrr.dest    = my_node;
  prrr.mi      = pnr.mi;
  prrr.channel = pnr.channel;
  prrr.seq     = pnr.seq;
  prrr.pages   = pnr.dist_pages;
  prrr.size    = pnr.dist_size;

  /************************************************************/
  /* Send data                                                */
  /************************************************************/

#ifdef _WITH_DDSCP
#ifndef _MULTIPLE_CPA
  pnr.copy_phys_addr_high        = copy_phys_addr_high;
  pnr.backup_copy_phys_addr_high = backup_copy_phys_addr_high;
#endif
#endif

#ifndef _WITH_DDSCP

  lpe_entry.routing_part = dest;

  /* We probably should add the first MI ?! */
  lpe_entry.control     = (*pending_receive)[pr].mi | LRM_ENABLE_MASK;
  lpe_entry.page_length = sizeof(received_receive_entry_t);
  lpe_entry.PLSA        = LOCALDATA2PHYS(&(*pending_receive)[pr].
					 received_receive);
  lpe_entry.PRSA        = (caddr_t) (mi_id +
				     REMOTETABLE2PHYS(received_receive, dest));

  if (!lpe_entry.page_length) {
    /* Should not happen */
    TRY2("put_add_entry_3")
    log(LOG_ERR,
	"slrpp_recv(): page length error on put_add_entry_3\n");
  }

  /* Initiate a DMA to the remote letter-box that will contain informations
     about the receive call. */
  TRY2("put_add_entry 8")
  res = put_add_entry(minor,
		      &lpe_entry);
  if (res) {
    /* Should not happen */
    log(LOG_ERR,
	"receive(): unrecoverable error\n");
    splx(s);
    return res;
  }

#ifndef _MULTIPLE_CPA

  /************************************************************/
  /* Do we need to send 1 or 2 pages to transmit the description
     of physical buffers ready to receive data ? */

  if (copy_phys_addr_high < backup_copy_phys_addr_high &&
      copy_phys_addr_high) {
    /* 2 pages */

    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
      (COPY_PHYS_ADDR_SIZE - backup_copy_phys_addr_high);
    lpe_entry.PLSA        = LOCALDATA2PHYS(*copy_phys_addr +
					   backup_copy_phys_addr_high);
    lpe_entry.PRSA        = (caddr_t) (free_range +
				       REMOTETABLE2PHYS(recv_phys_addr, dest));

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_4")
      log(LOG_ERR,
	  "slrpp_recv(): page length error on put_add_entry_4\n");
    }

    TRY2("put_add_entry 9")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      /* Should not happen */
      log(LOG_ERR,
	  "receive(): unrecoverable error\n");
      splx(s);
      return res;
    }

    /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
       it is required when using PCI-DDC */
    lpe_entry.control    |= LMP_MASK | NOS_MASK | NOR_MASK | LMI_ENABLE_MASK;
    lpe_entry.PRSA        = lpe_entry.PRSA + lpe_entry.page_length;
    lpe_entry.PLSA        = LOCALDATA2PHYS(copy_phys_addr);
    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
                            copy_phys_addr_high;

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_5")
      log(LOG_ERR,
	  "slrpp_recv(): page length error on put_add_entry_5\n");
    }

    TRY2("put_add_entry 10")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      /* Should not happen */
      log(LOG_ERR,
	  "receive(): unrecoverable error\n");
      splx(s);
      return res;
    }

  } else {
    /* 1 page */

    /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
       it is required when using PCI-DDC */
    lpe_entry.control    |= LMP_MASK | NOS_MASK | NOR_MASK | LMI_ENABLE_MASK;
    lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) *
      ((copy_phys_addr_high ? copy_phys_addr_high : COPY_PHYS_ADDR_SIZE)
       - backup_copy_phys_addr_high);
    lpe_entry.PRSA        = (caddr_t) (free_range +
				       REMOTETABLE2PHYS(recv_phys_addr, dest));
    lpe_entry.PLSA        = LOCALDATA2PHYS(*copy_phys_addr +
					   backup_copy_phys_addr_high);

    if (!lpe_entry.page_length) {
      /* Should not happen */
      TRY2("put_add_entry_6")
      log(LOG_ERR,
	  "slrpp_recv(): page length error on put_add_entry_6\n");
    }

    TRY2("put_add_entry 11")
    res = put_add_entry(minor,
			&lpe_entry);
    if (res) {
      /* Should not happen */
      log(LOG_ERR,
	  "receive(): unrecoverable error\n");
      splx(s);
      return res;
    }
  }

#else

  /* Sending 1 page */
  /* LMI_ENABLE_MASK is not useful while using the emulator mode, but
     it is required when using PCI-DDC */
  lpe_entry.control    |= LMP_MASK | NOS_MASK | NOR_MASK | LMI_ENABLE_MASK;
  lpe_entry.page_length = sizeof(copy_phys_addr_entry_t) * size /* size == pnr.size */;
  lpe_entry.PRSA        = (caddr_t) (free_range +
				     REMOTETABLE2PHYS(recv_phys_addr, dest));
  lpe_entry.PLSA        = LOCALDATA2PHYS(pnr.pages);

  if (!lpe_entry.page_length) {
    /* Should not happen */
    TRY2("put_add_entry_7")
      log(LOG_ERR,
	  "slrpp_recv(): page length error on put_add_entry_7\n");
  }

  TRY2("put_add_entry 12")
  res = put_add_entry(minor,
		      &lpe_entry);
  if (res) {
    /* Should not happen */
    log(LOG_ERR,
	"receive(): unrecoverable error\n");
    splx(s);
    return res;
  }

#endif

  splx(s);
  return 0;

#else

  try_recv((*pending_receive) + pr);

  splx(s);
  return 0;

#endif
}


/*
 ************************************************************
 * slrpp_set_last_callback(dest, channel, fct, param, proc) :
 * set the callback function called at channel close time.
 * dest    : remote node.
 * channel : channel identifier.
 * fct     : callback function.
 * param   : param to provide the function with.
 *
 * return values : 0           : success.
 ************************************************************
 */

int
slrpp_set_last_callback(dest, channel, fct, param, proc)
                        node_t dest;
			channel_t channel;
			boolean_t (*fct) __P((caddr_t));
			caddr_t param;
			struct proc *proc;
{
  last_callback[NODE2NODEINDEX(dest, my_node)][channel].fct   = fct;
  last_callback[NODE2NODEINDEX(dest, my_node)][channel].param = param;

  return 0;
}


/*
 ************************************************************
 * slrpp_open_channel(dest, channel, classname, proc) : open a channel.
 * dest      : remote node.
 * channel   : channel identifier.
 * classname : classname associated with this channel.
 * proc      : proc structure of the calling process.
 *
 * return values : 0           : success.
 *                 ERANGE      : channel or dest node out of range.
 *                 ESHUTDOWN   : channel is being shutdown.
 *                 EISCONN     : channel already opened.
 *                 EFAULT      : channel can't be opened.
 ************************************************************
 */

int
slrpp_open_channel(dest, channel, classname, proc)
                   node_t dest;
		   channel_t channel;
		   appclassname_t classname;
		   struct proc *proc;
{
  int s;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (channel < MIN_REALLOC_CHANNEL)
    return EINVAL;

  s = splhigh();

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_OPEN) {
    splx(s);
    return EISCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  channel_state[NODE2NODEINDEX(dest, my_node)]
               [channel] = CHAN_OPEN;

  channel_classname[NODE2NODEINDEX(dest, my_node)]
                   [channel] = classname;

  last_callback[NODE2NODEINDEX(dest, my_node)]
               [channel].fct = NULL;
  last_callback[NODE2NODEINDEX(dest, my_node)]
               [channel].param = NULL;

  splx(s);

  return 0;
}


/*
 ************************************************************
 * slrpp_shutdown0_channel(dest, channel, seq_send, seq_recv, proc) :
 * shutdown a channel.
 * dest     : remote node.
 * channel  : channel identifier.
 * seq_send : variable ready to contain sequence number to reach.
 * seq_recv : variable ready to contain sequence number to reach.
 * proc     : proc structure of the calling process.
 *
 * return values : 0           : success.
 *                 ERANGE      : channel or dest node out of range.
 *                 ESHUTDOWN   : channel is being shutdown.
 *                 ENOTCONN    : channel closed.
 *                 EFAULT      : channel can't be shutdown.
 ************************************************************
 */

int
slrpp_shutdown0_channel(dest, channel, seq_send, seq_recv, proc)
                       node_t dest;
		       channel_t channel;
		       seq_t *seq_send, *seq_recv;
		       struct proc *proc;
{
  int s, i;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (channel < MIN_REALLOC_CHANNEL)
    return EFAULT;

  s = splhigh();

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  channel_state[NODE2NODEINDEX(dest, my_node)]
               [channel] = CHAN_SHUTDOWN_0;

  *seq_send = send_seq[NODE2NODEINDEX(dest, my_node)]
                      [channel].seq;
  *seq_recv = recv_seq[NODE2NODEINDEX(dest, my_node)]
                      [channel].seq;

  /* Prevent the callback functions to be invoked */
  for (i = 0; i < PENDING_SEND_SIZE; i++)
    if (pending_send[i].valid != INVALID &&
	pending_send[i].channel == channel &&
	pending_send[i].dest == dest) {
      pending_send[i].fct = NULL;
      pending_send[i].param = NULL;
    }

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
    if ((*pending_receive)[i].valid != INVALID &&
	(*pending_receive)[i].channel == channel &&
	(*pending_receive)[i].sender == dest) {
      (*pending_receive)[i].fct = NULL;
      (*pending_receive)[i].param = NULL;
    }

  splx(s);

  return 0;
}


/*
 ************************************************************
 * slrpp_shutdown1_channel(dest, channel, seq_send, seq_recv, proc) :
 * shutdown a channel.
 * dest     : remote node.
 * channel  : channel identifier.
 * seq_send : max sequence number to reach.
 * seq_recv : max sequence number to reach.
 * proc     : proc structure of the calling process.
 *
 * return values : 0         : success.
 *                 ERANGE    : channel or dest node out of range.
 *                 ESHUTDOWN : channel is being shutdown.
 *                 ENOTCONN  : channel closed.
 *                 EFAULT    : channel can't be shutdown.
 *                 EISCONN   : channel still opened.
 *                 ...       : slrp_send/recv() error codes.
 ************************************************************
 */

int
slrpp_shutdown1_channel(dest, channel, seq_send, seq_recv, proc)
                        node_t dest;
			channel_t channel;
			seq_t seq_send, seq_recv;
			struct proc *proc;
{
  int s;
  int res;
  seq_t seq;
  pagedescr_t pdescr;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (channel < MIN_REALLOC_CHANNEL)
    return EFAULT;

  s = splhigh();

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_OPEN) {
    splx(s);
    return EISCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_2) {
    splx(s);
    return ESHUTDOWN;
  }

  channel_state[NODE2NODEINDEX(dest, my_node)]
               [channel] = CHAN_SHUTDOWN_1;

  pdescr.addr = (caddr_t) vtophys(&wired_trash_data);
  pdescr.size = sizeof wired_trash_data;

  for (seq = send_seq[NODE2NODEINDEX(dest, my_node)][channel].seq;
       seq != seq_send;
       seq++) {
    res = _slrpp_send(dest, channel, &pdescr, 1,
		      NULL, NULL, NULL, proc, TRUE);
    if (res != 0) {
      splx(s);
      log(LOG_ERR, "shutdown channel send: fatal error\n");
      return res;
    }
  }

  for (seq = recv_seq[NODE2NODEINDEX(dest, my_node)][channel].seq;
       seq != seq_recv;
       seq++) {
    res = _slrpp_recv(dest, channel, &pdescr, 1,
		      NULL, NULL, NULL, proc, TRUE);
    if (res != 0) {
      splx(s);
      log(LOG_ERR, "shutdown channel recv: fatal error\n");
      return res;
    }
  }

  channel_state[NODE2NODEINDEX(dest, my_node)]
               [channel] = CHAN_SHUTDOWN_2;

  splx(s);

  return 0;
}


/*
 ************************************************************
 * slrpp_close_channel(dest, channel, proc) :
 * close a channel.
 * dest     : remote node.
 * channel  : channel identifier.
 * proc     : proc structure of the calling process.
 *
 * return values : 0           : success.
 *                 EAGAIN      : channel being closed, try again to check
 *                               for completion.
 *                 ERANGE      : channel or dest node out of range.
 *                 ESHUTDOWN   : channel is being shutdown.
 *                 ENOTCONN    : channel already closed.
 *                 EISCONN     : channel opened.
 *                 EFAULT      : channel can't be closed.
 ************************************************************
 */

int
slrpp_close_channel(dest, channel, proc)
                    node_t dest;
		    channel_t channel;
		    struct proc *proc;
{
  int i, s, res;

  if (_PNODE2NODE(dest) >= MAX_NODES)
    return ERANGE;

  if (channel >= CHANNEL_RANGE)
    return ERANGE;

  if (channel < MIN_REALLOC_CHANNEL)
    return EFAULT;

  s = splhigh();

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_CLOSE) {
    splx(s);
    return ENOTCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_OPEN) {
    splx(s);
    return EISCONN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_0) {
    splx(s);
    return ESHUTDOWN;
  }

  if (channel_state[NODE2NODEINDEX(dest, my_node)]
                   [channel] == CHAN_SHUTDOWN_1) {
    splx(s);
    return ESHUTDOWN;
  }

  for (i = 0; i < PENDING_SEND_SIZE; i++) {
    if (pending_send[i].valid != INVALID &&
	pending_send[i].channel == channel &&
	pending_send[i].dest == dest) {
      splx(s);
      return EAGAIN;
    }
  }

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++) {
    if ((*pending_receive)[i].valid != INVALID &&
	(*pending_receive)[i].channel == channel &&
	(*pending_receive)[i].sender == dest) {
      splx(s);
      return EAGAIN;
    }
  }
  /* No pending receive here, then no active RFCP nor MICP process
     for this channel. */

#ifdef _WITH_DDSCP
  /* Check that there is no active SFCP process for this channel. */
  if (SFCP_check_channel(minor, dest, channel) == TRUE) {
    splx(s);
    return EAGAIN;
  }
#endif

  /* Call the last callback function */
  if (last_callback[NODE2NODEINDEX(dest, my_node)]
                   [channel].fct != NULL) {
    res = last_callback[NODE2NODEINDEX(dest, my_node)]
                       [channel].fct(last_callback[NODE2NODEINDEX(dest, my_node)]
                                                  [channel].param);
    if (res == FALSE) {
      splx(s);
      return EAGAIN;
    }
  }

  /* Final clean-up of the channel */

  last_callback[NODE2NODEINDEX(dest, my_node)]
               [channel].fct = NULL;
  last_callback[NODE2NODEINDEX(dest, my_node)]
               [channel].param = NULL;

  slrpp_reset_channel(dest, channel);
  channel_classname[NODE2NODEINDEX(dest, my_node)]
                   [channel] = ~0UL;
  channel_protocol[NODE2NODEINDEX(dest, my_node)]
                  [channel] = -1;

  channel_state[NODE2NODEINDEX(dest, my_node)][channel] = CHAN_CLOSE;

  splx(s);

  return 0;
}

