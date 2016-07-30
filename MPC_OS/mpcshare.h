
#ifndef _MPCSHARE_H_
#define _MPCSHARE_H_

/* $Id: mpcshare.h,v 1.4 2000/03/08 21:45:23 alex Exp $ */

#ifdef _SMP_SUPPORT
#ifndef KERNEL
#include <sys/ioctl.h>
#endif
#else
#include <sys/ioctl.h>
#endif

#include <sys/types.h>

#define HSL_DEVICE "/dev/hsl"

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b)) 
#endif
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

typedef u_short seq_t;

/* If this enumeration is to be changed, protocol_names[] in modules/driver.c
   must be updated */
enum { HSL_PROTO_PUT,
       HSL_PROTO_SLRP_P, HSL_PROTO_SCP_P,
       HSL_PROTO_SLRP_V, HSL_PROTO_SCP_V,
       HSL_PROTO_MDCP };

extern char **environ;

typedef u_short pnode_t;
typedef u_short channel_t;

#define MAX_FD 256

// MAX_ALLOC_CHANNEL must be less or equal than CHANNEL_RANGE in modules/data.h
// MIN_ALLOC_CHANNEL must be equal or higher than MIN_REALLOC_CHANNEL
// in modules/data.h
#define MIN_ALLOC_CHANNEL 200
#define MAX_ALLOC_CHANNEL /*0xFFFF*/ 250

#define MIN_RESERVED_APPCLASS 0L
#define MAX_RESERVED_APPCLASS 255L
#define MIN_SUSER_APPCLASS    (MAX_RESERVED_APPCLASS + 1)
#define MAX_SUSER_APPCLASS    2047L
#define MIN_USER_APPCLASS     (MAX_SUSER_APPCLASS + 1)
#define MAX_USER_APPCLASS     /*0x10000L*/ 0x2000L

#define APPCLASS_INTERNET     MIN_SUSER_APPCLASS

/* __progname is set by crt0.o */
extern char *__progname;

typedef u_long appclassname_t;

typedef unsigned long long int primary_t;

typedef union _appsubclassname {
  u_short is_raw; /* value 0xFFFF if raw */
  struct {
    u_short is_raw; /* value 0xFFFF if raw */
    u_short secondary;
    primary_t primary;
  } raw;
  struct {
    u_short prefnode_cluster; /* value 0xFFFF is forbidden */
    pnode_t prefnode_pnode;
    primary_t value;
  } controlled;
} appsubclassname_t;

#ifdef __cplusplus
/* To instanciate pair templates with appsubclassname_t,
   we need a comparator. */
extern bool operator < (const appsubclassname_t &, const appsubclassname_t &);
#endif

/*
 * _IOW : export data from library to daemon (IOC_IN)
 * _IOR : import data from daemon to library (IOC_OUT)
 */

#define MPCMSGMAKEAPPCLASS _IOR('M', 6, appclassname_t)

typedef struct _mpcmsg_get_channel {
  appclassname_t mainclass;      /* OUT library -> daemon */
  appsubclassname_t subclass;    /* OUT */
  int protocol;                  /* OUT */
  channel_t channel_pair_0;      /* IN */
  channel_t channel_pair_1;      /* IN */
  int status;                    /* IN */
} mpcmsg_get_channel_t;
#define MPCMSGGETCHANNEL _IOWR('M', 8, mpcmsg_get_channel_t)

typedef struct _mpcmsg_close_channel {
  appclassname_t mainclass;      /* OUT library -> daemon */
  appsubclassname_t subclass;    /* OUT */
  int status;                    /* IN */
} mpcmsg_close_channel_t;
#define MPCMSGCLOSECHANNEL _IOWR('M', 9, mpcmsg_close_channel_t)

typedef struct _mpcmsg_get_local_infos {
  u_short cluster;               /* IN daemon -> library */
  pnode_t pnode;                 /* IN */
  int nclusters;                 /* IN */
} mpcmsg_get_local_infos_t;
#define MPCMSGGETLOCALINFOS _IOR('M', 10, mpcmsg_get_local_infos_t)

typedef struct _mpcmsg_get_node_count {
  u_short cluster;               /* OUT library -> daemon */
  int node_count;                /* IN */
} mpcmsg_get_node_count_t;
#define MPCMSGGETNODECOUNT _IOWR('M', 11, mpcmsg_get_node_count_t)

#define CMDLINE_SIZE 256
typedef struct _mpcmsg_spawn_task {
  char cmdline[CMDLINE_SIZE];    /* OUT library -> daemon */
  u_short cluster;               /* OUT */
  pnode_t pnode;                 /* OUT */
  appclassname_t mainclass;      /* OUT */
  int status;                    /* IN */
} mpcmsg_spawn_task_t;
#define MPCMSGSPAWNTASK _IOWR('M', 12, mpcmsg_spawn_task_t)

typedef struct _mpcmsg_test {
  int val1;
  int val2;
} mpcmsg_test_t;
#define MPCMSGTEST _IOWR('M', 0, mpcmsg_test_t)

#if 0
typedef struct _mpcmsg_accept {
  appclassname_t mainclass;      /* OUT library -> daemon */
  appsubclassname_t subclass;    /* OUT */
  uid_t uid;                     /* IN */
  pnode_t node;                  /* IN */
  channel_t channel;             /* IN */
  int status;                    /* IN */
} mpcmsg_accept_t;
#define MPCMSGACCEPTB  _IOWR('M', 1, mpcmsg_accept_t)
#define MPCMSGACCEPTNB _IOWR('M', 2, mpcmsg_accept_t)
#endif

typedef struct _mpcmsg_myname {
  char name[32];                 /* OUT library -> daemon */
} mpcmsg_myname_t;
#define MPCMSGMYNAME _IOW('M', 5, mpcmsg_myname_t)

/************************************************************/

/* Maximum number of channels opened by a process. */
#define MAX_CHAN_PROC 20

typedef struct _mpc_chan_set {
  pnode_t   dest;
  channel_t channel;
  int /* boolean_t */ is_set; /* in_use */
  /*
   * The following field is only used by the hsl_select() and associated
   * functions to indicate that an input, ouput or exceptionnal condition
   * happened on this channel. hsl_select() uses this field the same
   * way select() uses the obits array.
   */
#define DATA_READY_IN (1<<0)
#define DATA_READY_OU (1<<1)
#define DATA_READY_EX (1<<2)
  int data_ready;
} mpc_chan_set_t;

typedef struct {
  int max_index; /* The max index used or -1 if empty. */
  mpc_chan_set_t chan_set[MAX_CHAN_PROC];
} mpc_chan_set;

#endif

