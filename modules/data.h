
/* $Id: data.h,v 1.9 2000/03/08 21:45:25 alex Exp $ */

#ifndef _DATA_H_
#define _DATA_H_

#include <vm/vm_param.h>

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

#include <sys/param.h> /* Needed when including sys/proc.h */
#define ACTUALLY_LKM_NOT_KERNEL
#include <sys/proc.h>

#include <vm/vm.h>
#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <vm/lock.h>
#endif
#include <vm/pmap.h>
#include <machine/pmap.h>
#include <vm/vm_map.h>

/* Pour que AVOID_LINK_ERROR fonctionne, l'utilisateur de PUT
   doit mettre LMI s'il met LMP */
#ifdef AVOID_LINK_ERROR
#undef AVOID_LINK_ERROR
#endif
#define AVOID_LINK_ERROR

#ifdef _SMP_SUPPORT
#define PCI_COMMAND_STATUS_REG          0x04 
#define PCI_COMMAND_IO_ENABLE           0x00000001
#define PCI_COMMAND_MEM_ENABLE          0x00000002
#define PCI_COMMAND_MASTER_ENABLE       0x00000004
#define PCI_COMMAND_SPECIAL_ENABLE      0x00000008
#define PCI_COMMAND_INVALIDATE_ENABLE   0x00000010
#define PCI_COMMAND_PALETTE_ENABLE      0x00000020
#define PCI_COMMAND_PARITY_ENABLE       0x00000040
#define PCI_COMMAND_STEPPING_ENABLE     0x00000080
#define PCI_COMMAND_SERR_ENABLE         0x00000100
#define PCI_COMMAND_BACKTOBACK_ENABLE   0x00000200
#define PCI_STATUS_BACKTOBACK_OKAY      0x00800000
#define PCI_STATUS_PARITY_ERROR         0x01000000  
#define PCI_STATUS_DEVSEL_FAST          0x00000000
#define PCI_STATUS_DEVSEL_MEDIUM        0x02000000
#define PCI_STATUS_DEVSEL_SLOW          0x04000000
#define PCI_STATUS_DEVSEL_MASK          0x06000000
#define PCI_STATUS_TARGET_TARGET_ABORT  0x08000000
#define PCI_STATUS_MASTER_TARGET_ABORT  0x10000000
#define PCI_STATUS_MASTER_ABORT         0x20000000
#define PCI_STATUS_SPECIAL_ERROR        0x40000000
#define PCI_STATUS_PARITY_DETECT        0x80000000
#endif

#ifndef MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
# define MAX(a,b) (((a)>(b))?(a):(b)) 
#endif

#define MIN_SEQ(a,b) (SEQ_INF_STRICT((a),(b))?(a):(b))
#define MAX_SEQ(a,b) (SEQ_SUP_STRICT((a),(b))?(a):(b)) 

/*
 * Check if the intersection of [X1, X2] and [Y1, Y2] is empty (FALSE)
 * or not (TRUE)
 */
#define INSIDE_RANGE(X1, X2, Y1, Y2)  \
    (((X1) >= (Y1) && (X1) < (Y2)) || \
     ((X2) >= (Y1) && (X2) < (Y2)) || \
     ((X1) < (Y1) && (X2) >= (Y2)))

#ifndef _WITH_LOOPBACK
#define NOLOOPBACK 0
#define NODE2NODEINDEX(node, excl_node) \
                      (((node) < (excl_node)) ? (node) : ((node) - 1))
#else
#define NOLOOPBACK 1
#define NODE2NODEINDEX(node, excl_node) (node)
#endif

#define DEBUG_STAGE_SIZE 128
extern char debug_stage [DEBUG_STAGE_SIZE];
extern char debug_stage2[DEBUG_STAGE_SIZE];

/*
 * To examine the last TRY parameter, enter command "ex /s 0x..."
 * under the kernel debugger
 */
#define TRY(s)  { strncpy(debug_stage,  (s), sizeof(debug_stage)  - 1); }
#define TRY2(s) { strncpy(debug_stage2, (s), sizeof(debug_stage2) - 1); }

#define MAX_PCI_INT 2 /* Number of PCI interfaces on the same PCI bus */
#ifndef AVOID_LINK_ERROR
#define MAX_NODES_SIZE 4
                      /* Maximum number of nodes in the network */
#else
#define MAX_NODES_SIZE 2
#endif
#define MAX_NODES (1<<MAX_NODES_SIZE)

#define MI_SIZE  24 /* Maximum size */
#ifndef AVOID_LINK_ERROR
#define MI_RANGE 16
#define MI_RANGE_SLR 15 /* Size we actually use */
#else
#define MI_RANGE 8
#define MI_RANGE_SLR 7 /* Size we actually use */
#endif
#if MI_SIZE < MI_RANGE
#error MI_RANGE too big
#endif
#if MI_RANGE <= MI_RANGE_SLR
#error MI_RANGE_SLR too big
#endif

#if defined(AVOID_LINK_ERROR) && (MI_RANGE > (MI_SIZE - 16))
#error MI_RANGE too big
#endif

/* WARNING : if LPE_BURST is defined, HSL_LPE_SIZE must be a power of 2 */
/* In fact, HSL_LPE_SIZE should simply be a multiple of LPE_BURST_LIMIT,
   no power of 2 necessary */
#ifndef AVOID_LINK_ERROR
#define HSL_LPE_SIZE 1024
#define HSL_LMI_SIZE 1024
#else
#define HSL_LPE_SIZE 65536
#define HSL_LMI_SIZE 65536
#endif

#define HSL_LRM_SIZE (1<<MI_RANGE)

#define HSLPRI (PZERO + 4) /* Should be correctly adjusted... */

#define ever ;;

#define MAX_CHANNEL  0xFFFF
#define MAX_SEQUENCE 0xFFFF

/* typedef u_short channel_t; */
/* typedef u_short seq_t; */
typedef u_short node_t; /* up to 65536 nodes */
typedef u_long  mi_t;

#ifdef _WITH_MONITOR
typedef u_short pnode_t;
#endif

/*
 * If A is the sender and B the receiver,
 * B sends to A a message with the MI : MAKE_MI(TYPE_RECV, receiver node, id)
 * A sends to B a message with the MI : MAKE_MI(TYPE_SEND, sender node,   id)
 */

/*
 * <------24 bits------>
 * +-------------------+
 * |OFFSET|NODE|ID|TYPE| <-- Structure of a MI
 * +-------------------+
 * <-------------------> <-- MI_SIZE
 *    <----------------> <-- MI_RANGE
 *        <------------> <-- MI_RANGE_SLR
 *        <---->         <-- MAX_NODES_SIZE
 *             <-->      <-- MAX_ID_RANGE
 *         XXXX          <-- MI_NODE_MASK
 *              XX       <-- MI_ID_MASK
 *                 XXXX  <-- MI_TYPE_MASK
 */

#define MI_TYPE_MASK 1
#define MI_NODE_MASK ((MAX_NODES - 1)<<(MI_RANGE_SLR - MAX_NODES_SIZE))
#define MI_ID_MASK (((1<<(MI_RANGE_SLR - 1 - MAX_NODES_SIZE)) - 1)<<1)

#define MI_TYPE_PART(mi) ((mi) & MI_TYPE_MASK)
#define MI_NODE_PART(mi) (((mi) & MI_NODE_MASK)>>(MI_RANGE_SLR - MAX_NODES_SIZE))
#define MI_ID_PART(mi)   (((mi) & MI_ID_MASK)>>1)

#define MAKE_MI(type, node, id) (((type) & MI_TYPE_MASK) | ((id)<<1) | \
				 ((node)<<(MI_RANGE_SLR - MAX_NODES_SIZE)))

#define TYPE_SEND 0
#define TYPE_RECV 1

typedef enum { INVALID, VALID, LPE } valid_t;

/*
 * Data structures maintained by senders and receivers.
 */

/*
 * CHANNEL_RANGE must be equal (or higher) than MAX_ALLOC_CHANNEL
 * in MPC_OS/mpcshare.h
 * MIN_REALLOC_CHANNEL must be equal (or lower) than MIN_ALLOC_CHANNEL
 * in MPC_OS/mpcshare.h
 */
#define CHANNEL_RANGE       /*32*/ 250
#define MIN_REALLOC_CHANNEL  200

/* PENDING_SEND_SIZE must be copied in MPC_OS/distobj.cc */
#define PENDING_SEND_SIZE     200

#define SEND_PHYS_ADDR_SIZE   200
#ifndef AVOID_LINK_ERROR
#define MI_ID_RANGE           (1<<8)
#else
#define MI_ID_RANGE           (1<<3)
#endif
#if MI_ID_RANGE > 1<<(MI_RANGE_SLR - 1 - MAX_NODES_SIZE)
#error MI_ID_RANGE too big
#endif
#define RECEIVED_RECEIVE_SIZE MI_ID_RANGE
#define RECV_PHYS_ADDR_SIZE   200

/* PENDING_RECEIVE_SIZE must be copied in MPC_OS/distobj.c */
#define PENDING_RECEIVE_SIZE  200

#ifndef _MULTIPLE_CPA
#define COPY_PHYS_ADDR_SIZE   400
#endif

#define PAGESTAB_SIZE SEND_PHYS_ADDR_SIZE
#ifdef _MULTIPLE_CPA
#define INTERNAL_CPA_SIZE /*(PAGESTAB_SIZE + 1)*/ 20
#endif

#ifdef WITH_STABS_PCIBUS_SET
/* Maximum number of vm map entries a single SEND or RECV can overlap */
#define PROT_MAP_REF_TAB_SIZE 3
#endif

#ifdef WITH_STABS_PCIBUS_SET
typedef struct _prot_range {
  vm_offset_t start;
  size_t      size;
} prot_range_t;
#endif

typedef struct _pagedescr {
  caddr_t addr;
  size_t  size;
} pagedescr_t;

typedef struct _send_phys_addr_entry {
  caddr_t address;
  size_t  size;
  struct _send_phys_addr_entry *next;
  valid_t valid; /* values : INVALID/VALID */
} send_phys_addr_entry_t;

extern int send_phys_addr_alloc_entries;

typedef struct _send_seq_entry {
  seq_t seq;
#ifdef _WITH_DDSCP
  node_t SFCP_prev_node;
  channel_t SFCP_prev_channel;
  node_t SFCP_next_node;
  channel_t SFCP_next_channel;
  /* boolean_t */ int SFCP_inside_list;
#endif
} send_seq_entry_t;

typedef enum _terminate_type { SET_DEAD, CLEAR_DEAD } terminate_type_t;

/*
 obsolete : the manager is now in charge of closing the channel
typedef struct _received_receive_entry_terminate {
  node_t dest;
  mi_t   mi, mi_ref;
  terminate_type_t type;
} received_receive_entry_terminate_t;
*/

typedef struct _received_receive_entry_general {
  node_t dest;

  /* value is MAKE_MI(TYPE_RECV, receiver node, id) */
  mi_t   mi;

  channel_t channel;
  seq_t     seq;

  ptrdiff_t pages;
  size_t    size;
} received_receive_entry_general_t;

typedef union _received_receive_entry {
/* terminate is obsolete : the monitor is now in charge of closing
   the channel */
/* received_receive_entry_terminate_t terminate; */
  received_receive_entry_general_t general;
} received_receive_entry_t;

typedef struct _recv_valid_entry_terminate {
  valid_t valid; /* values : INVALID/VALID */
} recv_valid_entry_terminate_t;

typedef enum { DEAD, ALIVE } dead_t;

typedef struct _recv_valid_entry_general {
  valid_t valid; /* values : INVALID/VALID */
  dead_t  dead;
  size_t  size; /* byte count of data actually sent (sender may want
		 to send more bytes than receiver asked for, the min
		 of these two values is computed just before sending
		 data, and is given to the user within the callback
		 function call in the sender node - an appropriate
		 piggy back can provide the receiver node with this
		 information) */
} recv_valid_entry_general_t;

typedef union _recv_valid_entry {
  recv_valid_entry_terminate_t terminate;
  recv_valid_entry_general_t   general;
} recv_valid_entry_t;

typedef struct _recv_phys_addr_entry {
  caddr_t address;
  size_t  size;
} recv_phys_addr_entry_t;

/*
 * Data structures maintained by recipients.
 */

typedef struct _copy_phys_addr_entry {
  pagedescr_t pagedescr;
} copy_phys_addr_entry_t;

typedef struct _pending_receive_entry {
  node_t sender;

  /* value is MAKE_MI(TYPE_RECV, receiver node, id) */
  mi_t mi;

  channel_t channel;
  seq_t     seq;

#ifndef _MULTIPLE_CPA
  copy_phys_addr_entry_t *pages;
#else
  copy_phys_addr_entry_t pages[INTERNAL_CPA_SIZE];
#endif
  size_t    size;

  ptrdiff_t dist_pages;
  size_t    dist_size;

  struct proc *proc;

  void (*fct) __P((caddr_t));
  caddr_t param;

#ifdef _WITH_DDSCP
  u_short stage;            /* phase */
  u_short micp_seq;         /* sequence in the MICP retransmission process */

#ifndef _MULTIPLE_CPA
  int copy_phys_addr_high;
  int backup_copy_phys_addr_high;
#endif
#endif

#ifdef WITH_STABS_PCIBUS_SET
  prot_range_t prot_map_ref_tab[PROT_MAP_REF_TAB_SIZE];
#endif
  int post_treatment_lock;

  valid_t valid; /* values : INVALID/VALID */

  received_receive_entry_t received_receive;

  /* Can't use boolean_t type because this file may be included in
     user level applications */
  int reorder_interrupts;
} pending_receive_entry_t;

#ifdef _WITH_DDSCP
typedef enum _stage_name { STAGE_START,
			   /* STAGE_TRANSMIT, */
			   STAGE_MICP } stage_name_t;
#endif

typedef struct _recv_seq_entry {
  seq_t seq;
#ifdef _WITH_DDSCP
  node_t RFCP_prev_node;
  channel_t RFCP_prev_channel;
  node_t RFCP_next_node;
  channel_t RFCP_next_channel;
  /* boolean_t */ int RFCP_inside_list;
#endif
} recv_seq_entry_t;


typedef struct _pending_send_entry {
  node_t    dest;

  /* value is MAKE_MI(TYPE_RECV, receiver node, id) */
  mi_t      mi;

  channel_t channel;
  seq_t     seq;

  send_phys_addr_entry_t *pages;
  size_t    size;

  struct proc *proc;

  void (*fct) __P((caddr_t,
		   int));
  caddr_t param;
  size_t size_sent;

#ifdef WITH_STABS_PCIBUS_SET
  prot_range_t prot_map_ref_tab[PROT_MAP_REF_TAB_SIZE];
#endif
  int post_treatment_lock;

  received_receive_entry_t *timeout; /* Not used by DDSCP */
  valid_t valid; /* values : INVALID/VALID/LPE for SLR,
                             INVALID/VALID for SCP */
} pending_send_entry_t;

/* These values must match modules/Makefile.am */
#define SERIALMAJOR 120
#define CMEMMAJOR   121
#define DEVMAJOR    122

#endif

