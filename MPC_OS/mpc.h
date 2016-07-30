
#ifndef _MPCLIB_H_
#define _MPCLIB_H_

/* $Id: mpc.h,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <unistd.h> 

#include "mpcshare.h"

#if 0
#define MPC_PERROR(X) perror(X)
#else
#define MPC_PERROR(X) fprintf(mpc_stderr, "%s: %s\n", (X), strerror(errno));
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern int dont_exit;

extern void mpc_init  __P((void));
extern void mpc_close __P((void));

extern appclassname_t make_appclass   __P((void));
extern int            delete_appclass __P((appclassname_t));

extern appsubclassname_t make_subclass_prefnode __P((u_short, pnode_t,
						     primary_t));
extern appsubclassname_t make_subclass_raw      __P((primary_t));

extern int mpc_get_channel __P((appclassname_t, appsubclassname_t,
				channel_t *, channel_t *, int));
extern int mpc_close_channel __P((appclassname_t, appsubclassname_t));

extern int mpc_get_local_infos __P((u_short *, pnode_t *, int *));
extern int mpc_get_node_count __P((int));

extern int mpc_spawn_task __P((char *, u_short, pnode_t, appclassname_t));

extern int mpc_read  __P((pnode_t, channel_t, void *, size_t));
extern int mpc_write __P((pnode_t, channel_t, const void *, size_t));

#if 0
extern int mpc_accept_channel __P((appclassname_t, appsubclassname_t,
				   int, uid_t *, pnode_t *,
				   channel_t *));
#endif

extern void MPC_CHAN_SET   __P((pnode_t, channel_t, mpc_chan_set *));
extern void MPC_CHAN_CLR   __P((pnode_t, channel_t, mpc_chan_set *));
extern int  MPC_CHAN_ISSET __P((pnode_t, channel_t, mpc_chan_set *));
extern void MPC_CHAN_ZERO  __P((mpc_chan_set *));

extern int mpc_select __P((int, fd_set *, fd_set *, fd_set *,
			   mpc_chan_set *, mpc_chan_set *, mpc_chan_set *,
			   struct timeval *));

#endif

