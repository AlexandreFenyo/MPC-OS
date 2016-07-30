
/* $Id: ddslrpp.h,v 1.3 1999/06/10 17:28:51 alex Exp $ */

#ifndef _DDSLRPP_H_
#define _DDSLRPP_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <sys/select.h>

#include "data.h"
#include "driver.h"

#ifdef _WITH_DDSCP
#include "flowcp.h"
#endif

/* Timeout in ticks : see kern_clock.c && hzto().
   This value should be tuned later */
#define SLR_P_TIMEOUT_TICKS 1000

/* In the FreeBSD implementation of timeout(), the original arguments
   to timeout are used to identify entries for untimeout */
#define SLR_P_TIMEOUT_ARG 0xAFAF

typedef enum { CHAN_OPEN, CHAN_SHUTDOWN_0, CHAN_SHUTDOWN_1,
	       CHAN_SHUTDOWN_2, CHAN_CLOSE } channel_state_t;
extern channel_state_t channel_state[MAX_NODES - NOLOOPBACK][CHANNEL_RANGE];

extern u_long wired_trash_data;

extern appclassname_t channel_classname[MAX_NODES - NOLOOPBACK][CHANNEL_RANGE];
extern int channel_protocol[MAX_NODES - NOLOOPBACK][CHANNEL_RANGE];

extern struct selinfo channel_select_info_in[MAX_NODES - NOLOOPBACK]
                                            [CHANNEL_RANGE];
extern struct selinfo channel_select_info_ou[MAX_NODES - NOLOOPBACK]
                                            [CHANNEL_RANGE];
extern struct selinfo channel_select_info_ex[MAX_NODES - NOLOOPBACK]
                                            [CHANNEL_RANGE];
extern int channel_data_ready[MAX_NODES - NOLOOPBACK]
                             [CHANNEL_RANGE];

extern uid_t classname2uid[MAX_USER_APPCLASS];

extern node_t my_node;

typedef u_char nodes_tab_t[32];

extern send_seq_entry_t send_seq[MAX_NODES - NOLOOPBACK][CHANNEL_RANGE];
extern pending_receive_entry_t (*pending_receive)[PENDING_RECEIVE_SIZE];
extern received_receive_entry_t (*received_receive)[MAX_NODES - NOLOOPBACK]
                                                   [RECEIVED_RECEIVE_SIZE];
extern pending_send_entry_t pending_send[PENDING_SEND_SIZE];
extern recv_valid_entry_t recv_valid[MAX_NODES - NOLOOPBACK][RECEIVED_RECEIVE_SIZE];
extern recv_seq_entry_t recv_seq[MAX_NODES - NOLOOPBACK][CHANNEL_RANGE];

extern boolean_t timeout_in_progress;
#ifdef _SMP_SUPPORT
extern struct callout_handle timeout_handle;
#endif

extern int pending_send_free_ident;
extern int send_phys_addr_free_ident;

extern vm_offset_t contig_space;
extern caddr_t     contig_space_phys;

extern boolean_t SEQ_INF_STRICT __P((seq_t, seq_t));
extern boolean_t SEQ_SUP_STRICT __P((seq_t, seq_t));
extern boolean_t SEQ_INF_EQUAL  __P((seq_t, seq_t));
extern boolean_t SEQ_SUP_EQUAL  __P((seq_t, seq_t));

extern int init_slr __P((int));

extern void end_slr __P((void));

extern void close_slr __P((void));

extern void slr_clear_config __P((void));

extern int set_config_slr __P((slr_config_t));

extern nodes_tab_t *slrpp_get_nodes __P((void));

extern boolean_t slrpp_cansend __P((node_t,
				    channel_t));

extern __inline int slrpp_send __P((node_t,
				    channel_t,
				    pagedescr_t *,
				    size_t,
				    void (*) __P((caddr_t, int)),
				    caddr_t,
				    struct proc *));

extern int _slrpp_send __P((node_t,
 		 	    channel_t,
	 		    pagedescr_t *,
			    size_t,
			    void (*) __P((caddr_t, int)),
			    caddr_t,
			    pending_send_entry_t **,
			    struct proc *,
			    boolean_t));

extern __inline int slrpp_recv __P((node_t,
				    channel_t,
				    pagedescr_t *,
				    size_t,
				    void (*) __P((caddr_t)),
				    caddr_t,
				    struct proc *));

extern int _slrpp_recv __P((node_t,
			    channel_t,
			    pagedescr_t *,
			    size_t,
			    void (*) __P((caddr_t)),
			    caddr_t,
			    pending_receive_entry_t **,
			    struct proc *,
			    boolean_t));

extern void slrpp_reset_channel __P((node_t,
                                     channel_t));

extern boolean_t slrpp_channel_check_rights __P((node_t,
						 channel_t,
						 struct proc *));

extern void slrpp_set_appclassname __P((appclassname_t,
					uid_t));

extern void ddslrp_timeout __P((void *));

#ifdef _WITH_DDSCP
extern int try_recv              __P((pending_receive_entry_t *));
extern void copy_phys_update     __P((pending_receive_entry_t *));
extern boolean_t SLRP_check_jobs __P((void));
extern boolean_t SLRP_do_jobs    __P((int));
#endif

extern int slrpp_set_last_callback __P((node_t,
					channel_t,
					boolean_t (*) __P((caddr_t)),
					caddr_t,
					struct proc *));

extern int slrpp_open_channel     __P((node_t,
				       channel_t,
				       appclassname_t,
				       struct proc *));
extern int slrpp_shutdown0_channel __P((node_t,
					channel_t,
					seq_t *,
					seq_t *,
					struct proc *));
extern int slrpp_shutdown1_channel __P((node_t,
					channel_t,
					seq_t,
					seq_t,
					struct proc *));
extern int slrpp_close_channel    __P((node_t,
				       channel_t,
				       struct proc *));

#endif

