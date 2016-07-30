
/* $Id: mdcp.h,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#ifndef _MDCP_H_
#define _MDCP_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

typedef struct _mdcp_param_info {
  node_t    dest;
  channel_t main_channel;
  channel_t bufferized_channel;
  boolean_t blocking;
  boolean_t input_buffering;
  size_t    input_buffering_maxsize; /* 0 means infinite. */
  boolean_t output_buffering;
} mdcp_param_info_t;

/*
 * Available possibilities :
 *
 * buffering : INPUT OUTPUT
 * ------------------------
 *              NO     NO
 *              NO    YES
 *              YES   YES
 *
 * case not available : YES[input] / NO[output] -
 * this case is treated like YES[input] / YES[output].
 */

extern int mdcp_init __P((void));
extern int mdcp_end  __P((void));

extern int mdcp_init_com __P((node_t, channel_t, channel_t,
			      appclassname_t, struct proc *));
extern int mdcp_shutdown0_com __P((node_t, channel_t, seq_t *, seq_t *,
				   seq_t *, seq_t *, struct proc *));
extern int mdcp_shutdown1_com __P((node_t, channel_t, seq_t, seq_t,
				   seq_t, seq_t, struct proc *));
extern int mdcp_end_com __P((node_t, channel_t, struct proc *));

extern int mdcp_getparams __P((node_t, channel_t, mdcp_param_info_t *));
extern int mdcp_setparam_blocking __P((node_t, channel_t, boolean_t));
extern int mdcp_setparam_input_buffering __P((node_t, channel_t, boolean_t));
extern int mdcp_setparam_input_buffering_maxsize __P((node_t, channel_t, size_t));
extern int mdcp_setparam_output_buffering __P((node_t, channel_t, boolean_t));

extern int mdcp_write __P((node_t, channel_t, caddr_t, size_t,
			   size_t *, struct proc *));
extern int mdcp_read __P((node_t, channel_t, caddr_t, size_t,
			  size_t *, struct proc *));

extern int hsl_select __P((struct proc *, libmpc_select_t *, int *));

#endif

