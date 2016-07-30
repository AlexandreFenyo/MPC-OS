
/* $Id: driver.h,v 1.7 2000/02/23 18:11:03 alex Exp $ */

#ifndef _DRIVER_H_
#define _DRIVER_H_

#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif
#include "put.h"

/************************************************************/
/* PCI Configuration */

#define HSL_PCI_BUS 0
#define HSL_PCI_FUNC 0
#define DEVICEID_VENDORID 0
#define DEVICEID_VENDORID_PCIDDCVALUE 0x0
#define HSL_MAX_DEV 32

#ifndef CONF1_ADDR_PORT
#define CONF1_ADDR_PORT 0x0cf8
#endif

#ifndef CONF1_DATA_PORT
#define CONF1_DATA_PORT 0x0cfc
#endif

/* PCIDDC configuration space */

#define PCIDDC_STATUS 0x40
#define PCIDDC_STATUS_PAGE_TRANSMITTED          0x80000000UL
#define PCIDDC_STATUS_PAGE_RECEIVED             0x40000000UL
#define PCIDDC_STATUS_PAGE_TRANSMITTED_4096     0x20000000UL
#define PCIDDC_STATUS_PAGE_TRANSMITTED_256      0x10000000UL
#define PCIDDC_STATUS_PAGE_TRANSMITTED_16       0x08000000UL
#define PCIDDC_STATUS_PAGE_RECEIVED_4096        0x04000000UL
#define PCIDDC_STATUS_PAGE_RECEIVED_256         0x02000000UL
#define PCIDDC_STATUS_PAGE_RECEIVED_16          0x01000000UL
#define PCIDDC_STATUS_EEP_ERROR                 0x00800000UL
#define PCIDDC_STATUS_CRC_HEADER_ERROR          0x00400000UL
#define PCIDDC_STATUS_CRC_DATA_ERROR            0x00200000UL
#define PCIDDC_STATUS_EP_ERROR                  0x00100000UL
#define PCIDDC_STATUS_TIMEOUT_ERROR             0x00080000UL
#define PCIDDC_STATUS_R3_STATUS_ERROR           0x00040000UL
#define PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR 0x00020000UL
#define PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR    0x00010000UL
#define PCIDDC_STATUS_SENT_PACKET_OF_ERROR      0x00008000UL
#define PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR  0x00004000UL
#define PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR  0x00002000UL
#define PCIDDC_STATUS_LMI_OF_ERROR              0x00001000UL
#define PCIDDC_STATUS_R3_STATUS_MASK            0x000000FFUL
#define PCIDDC_STATUS_TRANSACTION_MASK          0x00000F00UL
#define PCIDDC_STATUS_IRQ_L7                    0x00000080UL
#define PCIDDC_STATUS_IRQ_L6                    0x00000040UL
#define PCIDDC_STATUS_IRQ_L5                    0x00000020UL
#define PCIDDC_STATUS_IRQ_L4                    0x00000010UL
#define PCIDDC_STATUS_IRQ_L3                    0x00000008UL
#define PCIDDC_STATUS_IRQ_L2                    0x00000004UL
#define PCIDDC_STATUS_IRQ_L1                    0x00000002UL
#define PCIDDC_STATUS_IRQ_L0                    0x00000001UL

#define PCIDDC_STATUS_MASK_REGISTER 0x44

#define PCIDDC_COMMAND 0x48
#define PCIDDC_SOFT_RESET 0x00008000UL
#define PCIDDC_Tx_INIT    0x00004000UL
#define PCIDDC_R3_INIT    0x00002000UL
#define PCIDDC_Rx_INIT    0x00001000UL
#define PCIDDC_LRM_ENABLE 0x00000200UL
#define PCIDDC_LOOPBACK   0x00000100UL
#define PCIDDC_MAX_PACKET_LENGTH_START 16
#define PCIDDC_MAX_PACKET_LENGTH_MASK  0x0FFF0000UL
#define PCIDDC_TXRX_BALANCE_START      28
#define PCIDDC_TXRX_BALANCE_MASK       0xF0000000UL

#define PCIDDC_LPE_MAX     0x50
#define PCIDDC_LPE_CURRENT 0x54
#define PCIDDC_LPE_NEW     0x58
#define PCIDDC_LPE_BASE    0x5C
#define PCIDDC_LMI_MAX     0x60
#define PCIDDC_LMI_CURRENT 0x64
#define PCIDDC_LMI_NEW     0x68
#define PCIDDC_LMI_BASE    0x6C
#define PCIDDC_LRM_BASE    0x70
#define PCIDDC_R3_DOUT     0x74

#ifndef _SMP_SUPPORT
struct pcibus *getpcibus __P((void));
#endif

/************************************************************/

#define ETHERNET_LPE_SIZE /*5*/ 512

/************************************************************/

typedef struct _driver_stats {
  /* Stats for SLR/P */

  /* number of calls to slrpp_send() */
  int calls_slrpp_send;
  /* number of calls to slrpp_recv() */
  int calls_slrpp_recv;

  /* number of calls to slrpv_send() */
  int calls_slrpv_send;
  /* number of calls to slrpv_recv() */
  int calls_slrpv_recv;

  /* number of calls to ddslrp_timeout() */
  int calls_timeout;
  /* number of timeouts started by timeout() */
  int timeout_timeout;

  /* Stats for PUT */

  /* number of calls to put_add_entry() */
  int calls_put_add_entry;
  /* number of hardware interrupts */
  int calls_put_interrupt;
  /* number of unattended interrupts */
  int unattended_interrupts;

} driver_stats_t;

extern driver_stats_t driver_stats;

extern int perfmon_invalid;
extern struct timeval perfmon_tv_start;
extern struct timeval perfmon_tv_end;
#define PERFMON_TV_STRING_LEN 80

#define PERFMON_START { perfmon_invalid = 0; \
                        microtime(&perfmon_tv_start); }

#define PERFMON_STOP(X) { microtime(&perfmon_tv_end); \
                          add_event_tv((X)); }

#ifdef AVOID_LINK_ERROR
extern int hsltestput_layer;
#endif

/*
 *************************************************************
 * Those IOCTLs are for testing                              *
 *************************************************************
 */

#define HSLTEST _IOWR('H', 1, int)

struct _virtualRegion {
  caddr_t start;
  size_t  size;
};

#define HSLVIRT2PHYSTST _IOWR('H', 2, struct _virtualRegion)

#define HSLPROTECTWIRE _IOW('H', 29, struct _virtualRegion)
#define HSLPROTECTDUMP _IO('H', 30)

struct _physicalWrite {
  caddr_t addr;
  int value;
};

#define HSLWRITEPHYS2 _IOWR('H', 3, struct _physicalWrite)

#define HSLTESTSLR   _IOW('H', 40, caddr_t)
#define HSLTESTSLR2  _IOW('H', 41, caddr_t)
#define HSLTESTSLRV  _IOW('H', 42, caddr_t)
#define HSLTESTSLRV2 _IOW('H', 43, caddr_t)

#define HSLTEST2SLRV  _IOW('H', 61, caddr_t)
#define HSLTEST2SLRV2 _IOW('H', 62, caddr_t)

#define HSLTESTTIMER _IOW('H', 63, int)

typedef struct _latenceslr {
  node_t dest;
  caddr_t data;
  size_t  size;
} latenceslr_t;
#define HSLTESTSLRVLAT  _IOW('H', 64, struct _latenceslr)
#define HSLTESTSLRV2LAT _IOW('H', 65, struct _latenceslr)

typedef struct _sendrandompageinfo {
  lpe_entry_t entry;
  u_long v0;
} sendrandompageinfo_t;
#define HSLSENDRANDOMPAGE _IOW('H', 66, sendrandompageinfo_t)

#define HSLSIMULATEEEP _IO('H', 67)

#define MAX_TESTPUTBENCH 16

#define HSLTESTPUTRECV1 _IOR('H', 44, mi_t)
#define HSLTESTPUTRECV2 _IO('H', 45)

#define HSLTESTPUTSEND0 _IOW('H', 46, lpe_entry_t)
#define HSLTESTPUTSEND1 _IOW('H', 52, lpe_entry_t)
#define HSLTESTPUTSEND2 _IO('H', 47)

#define HSLTESTPUTBENCH1 _IOW('H', 48, int)
#define HSLTESTPUTBENCH2 _IOWR('H', 49, int)

typedef struct _hsl2tty {
  caddr_t addr;
  node_t  dest;
  int     size;
} hsl2tty_t;
#define HSL2TTYINIT    _IOW('H', 55, int)
#define HSL2TTYRECV    _IOWR('H', 53, hsl2tty_t)
#define HSL2TTYSEND    _IOWR('H', 54, hsl2tty_t)
#define HSL2TTYCANSEND _IOWR('H', 56, int)

#define HSLTESTSLRVPROT  _IOW('H', 50, struct _virtualRegion)
#define HSLTESTSLRV2PROT _IOW('H', 51, struct _virtualRegion)

typedef struct _hsl_bench_latence {
  boolean_t first;
  int size;
  node_t dist_node;
  u_long dist_addr; /* physical address */
  u_long local_addr; /* physical address */
  caddr_t local_addr_virt; /* virtual address */
  int count;
  int wait_count;
  int result;
} hsl_bench_latence_t;
#define HSLBENCHLATENCE1 _IOWR('H', 59, hsl_bench_latence_t)

typedef struct _hsl_bench_throughput {
  boolean_t first;
  int size;
  node_t dist_node;
  u_long dist_addr; /* physical address */
  u_long local_addr; /* physical address */
  caddr_t local_addr_virt; /* virtual address */
  int count;
  int wait_count;
  int result;
} hsl_bench_throughput_t;
#define HSLBENCHTHROUGHPUT1 _IOWR('H', 60, hsl_bench_throughput_t)

/***************************************************************/

#define HSLNOTICESELECT _IO('H', 57)

typedef struct _hsl_put_init {
  int length;
  node_t node;
  void *routing_table;
} hsl_put_init_t;

#define HSLPUTINIT   _IOW ('H', 4, hsl_put_init_t)
#define HSLSLRPINIT  _IOWR('H', 5, caddr_t)
#define HSLPUTEND    _IO  ('H', 6)
#define HSLSLRPEND   _IO  ('H', 7)
#define HSLVIRT2PHYS _IOWR('H', 8, caddr_t)
#define HSLADDLPE    _IOW ('H', 9, lpe_entry_t)
#define HSLGETLPE    _IOR ('H', 10, lpe_entry_t)
#define HSLFLUSHLPE  _IO  ('H', 11)

#ifdef _WITH_MONITOR
#define HSLGETDTP    _IOW('H', 24, caddr_t)
#endif

typedef struct _slr_config {
  node_t node;
  caddr_t contig_space;
} slr_config_t;
#define HSLSETCONFIG _IOWR('H', 12, slr_config_t)

typedef struct _physical_read {
  caddr_t src; /* Physical address */
  caddr_t data;
  u_long  len;
 } physical_read_t;
#define HSLREADPHYS _IOWR('H', 13, physical_read_t)

typedef struct _physical_write {
  caddr_t dst; /* Physical address */
  caddr_t data;
  u_long  len;
} physical_write_t;
#define HSLWRITEPHYS _IOWR('H', 14, physical_write_t)

#define HSLSIMINT _IOW('H', 15, lpe_entry_t)

typedef struct _opt_contig_mem {
  u_char *ptr;
  size_t  size;
  u_char *phys;
} opt_contig_mem_t;
#define HSLGETOPTCONTIGMEM _IOWR('H', 16, opt_contig_mem_t)

#define HSLGETNODE _IOR('H', 17, int)

#define HSLVMINFO _IOW('H', 21, pid_t)

#define MPCOSGETOBJ _IOR('H', 22, vm_object_t)

/************************************************************/

#define MAX_EVENTS_STRING_SZ 80
#define MAX_EVENTS 10000

typedef struct _trace_var {
  /* number of allocated entries in pending_send[][] */
  int pending_send_entry;

  /* number of allocated entries in pending_receive[][] */
  int pending_receive_entry;

  /* number of allocated entries in send_phys_addr[] */
  int send_phys_addr;

  /* number of allocated entries in copy_phys_addr[] */
  int copy_phys_addr;

  /* channel 0 in/out bytes */
  int channel_0_in, channel_0_out;

  /* channel 0 sequences */
  int sequence_0_in, sequence_0_out;

  /* channel 1 in/out bytes */
  int channel_1_in, channel_1_out;

  /* channel 1 sequences */
  int sequence_1_in, sequence_1_out;
} trace_var_t;

typedef enum {
  TRACE_VAR,
  TRACE_STR
} trace_type_t;

typedef enum {
  TRACE_NO_TYPE,
  TRACE_INIT_DRIVER,
  TRACE_CLOSE_DRIVER, /* not handled */
  TRACE_INIT_PUT,
  TRACE_END_PUT,
  TRACE_INIT_SLR_P,
  TRACE_END_SLR_P,
  TRACE_INIT_SLR_V,
  TRACE_END_SLR_V,
  TRACE_DECR_PS_ENTRY,
  TRACE_INCR_PS_ENTRY,
  TRACE_DECR_PR_ENTRY,
  TRACE_INCR_PR_ENTRY,
  TRACE_DECR_SPA_ENTRY,
  TRACE_INCR_SPA_ENTRY,
  TRACE_DECR_CPA_ENTRY,
  TRACE_INCR_CPA_ENTRY,
  TRACE_CHAN_0_IN,
  TRACE_CHAN_0_OUT,
  TRACE_CHAN_1_IN,
  TRACE_CHAN_1_OUT
} trace_type_entry_t;

typedef union {
  trace_var_t var;
  char        str[MAX_EVENTS_STRING_SZ];
} trace_t;

typedef struct _trace_event {
  trace_type_t       type;
  trace_type_entry_t entry_type;
  
  int          index;
  trace_t      trace;

  struct timeval tv;
} trace_event_t;

#define INCR_VAR(var, type) { int s = splhigh();                       \
                              trace_var. ## var ## ++;                 \
                              add_event(TRACE_VAR, (type),             \
					(trace_t) trace_var);          \
                              splx(s); };

#define DECR_VAR(var, type) { int s = splhigh();                       \
                              trace_var. ## var ## --;                 \
                              add_event(TRACE_VAR, (type),             \
					(trace_t) trace_var);          \
                              splx(s); };

#define INCR_VAR_MULT(var, type, size) { int s = splhigh();            \
                                         trace_var. ## var += (size);  \
                                         add_event(TRACE_VAR, (type),  \
						   (trace_t)           \
						   trace_var);         \
                                         splx(s); };

#define DECR_VAR_MULT(var, type, size) { int s = splhigh();            \
                                         trace_var. ## var -= (size);  \
                                         add_event(TRACE_VAR, (type),  \
						   (trace_t)           \
						   trace_var);         \
                                         splx(s); };

#define ADD_EVENT(type)     { int s = splhigh();                       \
                              add_event(TRACE_VAR, (type),             \
                              (trace_t) trace_var);                    \
                              splx(s); };

extern trace_var_t   trace_var;
extern trace_event_t trace_events[MAX_EVENTS];

extern void init_events         __P((void));

extern void add_event           __P((trace_type_t,
				     trace_type_entry_t,
			             trace_t));

extern void add_event_string    __P((char *));
extern void add_event_tv        __P((char *));

extern trace_event_t *get_event __P((void));

#define HSLGETEVENT _IOR('H', 20, trace_event_t)

/************************************************************/

typedef struct _mpc_pci_conf {
  u_char bus;
  u_char device;
  u_char func;
  u_long reg;
  u_long data;
} mpc_pci_conf_t;

#define PCIREADCONF  _IOWR('H', 18, mpc_pci_conf_t)
#define PCIWRITECONF _IOW('H', 19, mpc_pci_conf_t)

#define PCISETBUS _IOW('H', 23, struct linker_set *)

#define PCIDDCGETTABLESINFO _IOR('H', 25, tables_info_t)

#define HSLSETMODEREAD  _IOWR('H', 26, set_mode_t)
#define HSLSETMODEWRITE _IOW('H', 27, set_mode_t)

#define DUMPINTERNALTABLES _IO('H', 28)
#define DUMPCHANNELSTATE _IO('H', 33)

#define SLRVGARBAGECOLLECT _IO('H', 31)

/*
 *************************************************************
 * Those IOCTLs are for the manager                          *
 *************************************************************
 */

typedef struct _manager_open_channel {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  appclassname_t classname;
  int protocol;
} manager_open_channel_t;

#define MANAGEROPENCHANNEL _IOW('H', 32, manager_open_channel_t)

typedef struct _manager_shutdown_1stStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  seq_t ret_seq_send_0;
  seq_t ret_seq_recv_0;
  seq_t ret_seq_send_1;
  seq_t ret_seq_recv_1;
  appclassname_t classname;
  int protocol;
} manager_shutdown_1stStep_t;

#define MANAGERSHUTDOWN1stSTEP _IOWR('H', 34, manager_shutdown_1stStep_t)

typedef struct _manager_shutdown_2ndStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  seq_t seq_send_0;
  seq_t seq_recv_0;
  seq_t seq_send_1;
  seq_t seq_recv_1;
  int protocol;
} manager_shutdown_2ndStep_t;

#define MANAGERSHUTDOWN2ndSTEP _IOW('H', 36, manager_shutdown_2ndStep_t)

typedef struct _manager_shutdown_3rdStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  int protocol;
} manager_shutdown_3rdStep_t;

#define MANAGERSHUTDOWN3rdSTEP _IOW('H', 37, manager_shutdown_3rdStep_t)

typedef struct _manager_set_appclassname {
  appclassname_t cn;
  uid_t uid;
} manager_set_appclassname_t;

#define MANAGERSETAPPCLASSNAME _IOW('H', 35, manager_set_appclassname_t)

/*
 *************************************************************
 * Those IOCTLs are for libmpc.a                             *
 *************************************************************
 */

typedef struct _libmpc_read {
  node_t    node;
  channel_t channel;
  void     *buf;
  size_t    nbytes; /* IN/OUT variable */
} libmpc_read_t;

#define LIBMPCREAD _IOWR('H', 38, libmpc_read_t)

typedef struct _libmpc_write {
  node_t      node;
  channel_t   channel;
  const void *buf;
  size_t      nbytes; /* IN/OUT variable */
} libmpc_write_t;

#define LIBMPCWRITE _IOWR('H', 39, libmpc_write_t)

typedef struct _libmpc_select {
  mpc_chan_set *mpcchanset_in, *mpcchanset_ou, *mpcchanset_ex;
  int mpcchanset_in_max_index, mpcchanset_ou_max_index, mpcchanset_ex_max_index;
  int nd;
  fd_set *in, *ou, *ex;
  struct timeval *tv;
  int retval;
} libmpc_select_t;

#define LIBMPCSELECT _IOWR('H', 58, libmpc_select_t)

#endif

