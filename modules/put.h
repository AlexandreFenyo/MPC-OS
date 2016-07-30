
/* $Id: put.h,v 1.4 1999/09/19 16:16:36 alex Exp $ */

#ifndef _PUT_H_
#define _PUT_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

#include "/sys/i386/isa/isa_device.h"
#include "/sys/pci/pcivar.h"
#include "/sys/pci/pcireg.h"
#ifndef _SMP_SUPPORT
#include "/sys/pci/pcibus.h"
#endif

#include "data.h"

#ifdef _WITH_MONITOR
#define _PNODE2NODE(X) (pnode2node[(X)])
#else
#define _PNODE2NODE(X) (X)
#endif

#ifdef AVOID_LINK_ERROR
extern int avoid_stage;
extern boolean_t avoid_MI_safe;
extern mi_t avoid_random_mi;
extern u_long avoid_v0;
extern boolean_t avoid_simulate;
#endif

typedef struct _set_mode {
  int max_nodes;
#ifdef _WITH_MONITOR
  pnode_t node;
#else
  node_t node;
#endif
  int val;
} set_mode_t;

extern int set_mode_read  __P((int, set_mode_t *));
extern int set_mode_write __P((int, set_mode_t *));

extern u_long hsl_conf_read __P((int, u_long));
extern void hsl_conf_write  __P((int, u_long, u_long));

#ifndef _SMP_SUPPORT
extern void put_set_device __P((int, pcici_t));
#else
extern void put_set_device __P((int, pcicfgregs));
#endif

typedef struct _tables_info {
  u_long  lpe_phys;
  caddr_t lpe_virt;
  int     lpe_size;

  u_long  lmi_phys;
  caddr_t lmi_virt;
  int     lmi_size;

  u_long  lrm_phys;
  caddr_t lrm_virt;
  int     lrm_size;
} tables_info_t;

extern void get_tables_info __P((int, tables_info_t *));

/* Optional contig. address space allocated by PUT for users of this layer */
/*#define OPTIONAL_CONTIG_RAM_SIZE (16 * 1024)*/
#define OPTIONAL_CONTIG_RAM_SIZE (1024 * 1024)

extern vm_offset_t opt_contig_space;
extern vm_size_t   opt_contig_size;
extern caddr_t     opt_contig_space_phys;
extern int         opt_slot;

/* Software fix for hardware bug : misaligned buffers */
extern vm_offset_t misalign_contig_space;
extern vm_size_t   misalign_contig_size;
extern caddr_t     misalign_contig_space_phys;
extern int         misalign_slot;
#ifdef PCIDDC2ndRun
/* The following indicator is not used by now, because
   we suppose that PCIDDC 2nd Run correctly handles
   misaligned buffers */
boolean_t          misalign_in_use;
#endif

#define MAX_SAP 6 /* Maximum count of upper Service Access Points */

extern int hslgetlpe_ident;
extern int hslfreelpe_ident;

typedef struct _lpe_entry {
  u_short page_length;
  u_short routing_part;

  u_long control;
#define NOR_MASK        (1<<31)         /* Notify Once Received */
#define LMP_MASK        (1<<30)         /* Last Message Page */
#define CM_MASK         (1<<29)         /* Composed Message */
#define LRM_ENABLE_MASK (1<<28)         /* LRM Enable */
/* IORP_MASK : obsolete, for compatibility only */
#define IORP_MASK       (1<<28)         /* In Order Received Packet */
#define LMI_ENABLE_MASK (1<<27)         /* LMI Enable */
#define SM_MASK         (1<<26)         /* Short Message */
#define NOS_MASK        (1<<25)         /* Notify Once Sent */
#define RESERVED_MASK   (1<<24)         /* Reserved */
#define MI_MASK    ((1<<MI_SIZE) - 1)   /* Message Identifier */
#define NOR_ISSET(control)        ((control) & NOR_MASK        ? 1 : 0)
#define LMP_ISSET(control)        ((control) & LMP_MASK        ? 1 : 0)
#define CM_ISSET(control)         ((control) & CM_MASK         ? 1 : 0)
#define LRM_ENABLE_ISSET(control) ((control) & LRM_ENABLE_MASK ? 1 : 0)
/* IORP_ISSET : obsolete, for compatibility only */
#define IORP_ISSET(control)       ((control) & IORP_MASK       ? 1 : 0)
#define LMI_ENABLE_ISSET(control) ((control) & LMI_ENABLE_MASK ? 1 : 0)
#define SM_ISSET(control)         ((control) & SM_MASK         ? 1 : 0)
#define NOS_ISSET(control)        ((control) & NOS_MASK        ? 1 : 0)
#define RESERVED_ISSET(control)   ((control) & RESERVED_MASK   ? 1 : 0)

#ifndef AVOID_LINK_ERROR
#define MI_PART(control)          ((mi_t) ((control) & MI_MASK))
#else
#define MI_PART(control)          ((mi_t) ((control) & ((1<<MI_RANGE) - 1)))
#endif

/*
 * Emulation of short messages on top of normal messages
 * (software fix for hardware bug) : we use 8 normal messages,
 *  each one containing a MI of the following form :
 *
 * <--------24bits--------->
 * <-1-> <--7-> <-4-> <-12->
 *  SM?   HOST   SEQ   DATA
 *
 * Then, we have 12 bits transported by each of 8 messages : we transport
 * 96 bits, this is just the size of the structure _sm_emu_buf that
 * contains the data we need to transfer.
 */

#define SM_EMU_SELECT_MASK 0x800000 /* One bit to indicate it's a SM */
#define SM_EMU_HOST_MASK   0x7f0000 /* 7 bits (host number) as a key to
				       aggregate the 8 received normal
				       messages as a unique SM -> we
				       assume there is at most 128 nodes
				       in the network */
#define SM_EMU_OFFSET_MASK 0x00f000
#define SM_EMU_DATA_MASK   0x000fff
#define SM_EMU_SELECT_ISSET(control) ((control) & SM_EMU_SELECT_MASK ? 1 : 0)

  caddr_t PRSA, PLSA;
} lpe_entry_t;

#ifdef AVOID_LINK_ERROR
typedef struct _interrupt_entry_t {
  int index;
  void (*fct)();
  u_long args[4];
  lpe_entry_t lpe_entry;
  boolean_t use_lpe;
} interrupt_entry_t;

typedef struct _interrupt_table_t {
  int maxentries;
  int nentries;
  interrupt_entry_t interrupt[0];
} interrupt_table_t;
#endif

typedef struct _sm_emu_buf {
  u_long control;
  u_long data1;
  u_long data2;
} sm_emu_buf_t;

typedef struct _lmi_entry {
  u_short packet_number;
  u_char r3_status;
  u_char reserved;
  u_long control;
  u_long data1;
  u_long data2;
} lmi_entry_t;

#ifdef AVOID_LINK_ERROR
extern lmi_entry_t *avoid_v0_entry;
extern lmi_entry_t *avoid_v0_entry_n;
extern lmi_entry_t *avoid_v0_entry_nn;
#endif

#define SM_PACK_DATA(D1, D2) \
  ((((u_long) (D1)) & 0x0000FFFFUL) | (((u_long) (D2))<<16))
#define SM_DATA_1(D) (((u_long) (D)) & 0x0000FFFFUL)
#define SM_DATA_2(D) (((u_long) (D))>>16)

#define SM_PACK_DATA_8_24(D1, D2) \
  ((((u_long) (D1)) & 0x000000FFUL) | (((u_long) (D2))<<8))
#define SM_DATA_1_8_24(D) (((u_long) (D)) & 0x000000FFUL)
#define SM_DATA_2_8_24(D) (((u_long) (D))>>8)

typedef struct _lrm_entry {
  u_short rpn; /* received packet number */
  u_short epn; /* expected packet number */
} lrm_entry_t;

typedef struct _put_info {
  lpe_entry_t *lpe;
  long nentries;

  boolean_t remote_hsl[MAX_NODES]; /* True if node should be contacted
				      via HSL, False if node should be
				      contacted via Ethernet */

  node_t node;
  long lpe_high; /* Next free LPE entry */
  long lpe_low;  /* Oldest used LPE entry */

  boolean_t in_use[MAX_SAP];

  void (*IT_sent[MAX_SAP]) __P((int,
				mi_t,
				lpe_entry_t));
  void (*IT_received[MAX_SAP]) __P((int,
				    mi_t,
				    u_long /* data1 */,
				    u_long /* data2 */));
  struct _mi_space {
    boolean_t in_use;
    mi_t   mi_start;
    u_long mi_size;
    struct _mi_space *prev;
    struct _mi_space *next;
  } mi_space[MAX_SAP];
  struct _mi_space *first_space;

  boolean_t hsl_found;
#ifndef _SMP_SUPPORT
  pcici_t hsl_tag;
#else
  pcicfgregs hsl_probe;
#endif

  int                hsl_contig_slot;
  vm_offset_t        hsl_contig_space;
  vm_offset_t        hsl_contig_space_backup;
  vm_size_t          hsl_contig_size;
  caddr_t            hsl_contig_space_phys;

  lpe_entry_t (*hsl_lpe)[HSL_LPE_SIZE];
  lmi_entry_t (*hsl_lmi)[HSL_LMI_SIZE];
  lrm_entry_t (*hsl_lrm)[HSL_LRM_SIZE];

  lpe_entry_t *lpe_softnew_virt;

  /*
   * Optimization : lpe_current/lpe_new are never read from the board :
   * they are updated each time an interrupt for PAGE TRANSMITTED is raised
   * or a put_add_entry() is performed.
   */
  lpe_entry_t *lpe_new_virt;
  lpe_entry_t *lpe_current_virt;

  lmi_entry_t *lmi_current_virt;

  boolean_t    lpe_loaded; /* True when a DMA is active */

  boolean_t    stop_put; /* empêcher la réentrance de put_add_entry() */
} put_info_t;

#ifdef AVOID_LINK_ERROR
extern put_info_t put_info[];

extern interrupt_table_t *interrupt_table_LPE;
extern interrupt_table_t *interrupt_table_LMI;
#endif

extern int put_init_SAP __P((void));

extern int put_end_SAP __P((void));

extern int put_init __P((int,
			 u_long,
			 node_t,
			 void *));

extern int put_end __P((int));

extern int put_get_node __P((int));

extern mi_t put_get_mi_start __P((int,
				  int));

extern long put_get_lpe_high __P((int));

/* extern long put_get_lpe_low __P((int)); */

extern long put_get_lpe_free __P((int));

extern int put_register_SAP __P((int,
				 void (*) __P((int, mi_t, lpe_entry_t)),
				 void (*) __P((int, mi_t, u_long, u_long))));

extern int put_unregister_SAP __P((int,
				   int));

extern int put_attach_mi_range __P((int,
				    int,
				    u_long));

extern int put_add_entry __P((int,
			      lpe_entry_t *));

#ifdef AVOID_LINK_ERROR
extern int _put_add_entry __P((int,
			       lpe_entry_t *));
#endif

extern int put_get_entry __P((int,
			      lpe_entry_t *));

extern int put_flush_entry __P((int));

extern int put_simulate_interrupt __P((int,
				       lpe_entry_t));

#ifdef _WITH_MONITOR
extern int put_get_dtp   __P((pnode_t *));
#endif

#ifdef _WITH_DDSCP
extern int put_flush_LRM __P((int,
			      mi_t));
#endif

extern void put_flush_lpe __P((int));

extern void put_flush_lmi __P((int));

#endif

