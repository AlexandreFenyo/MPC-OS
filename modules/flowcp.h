

/* $Id: flowcp.h,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#ifndef _FLOWCP_H_
#define _FLOWCP_H_

#ifdef _WITH_DDSCP

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

#include "data.h"
#include "driver.h"

#define send_seq_X send_seq
extern send_seq_entry_t send_seq_Y[MAX_NODES - 1][CHANNEL_RANGE];

extern recv_seq_entry_t recv_seq_X[MAX_NODES - 1][CHANNEL_RANGE];
extern recv_seq_entry_t recv_seq_Y[MAX_NODES - 1][CHANNEL_RANGE];

boolean_t flowcp_do_jobs __P((int));

int init_flowcp __P((int));
void end_flowcp __P((int));

void SFCP_start_process __P((int, node_t, channel_t));
void RFCP_start_process __P((int, node_t, channel_t));
void MICP_start_process __P((int, pending_receive_entry_t *));

extern boolean_t SFCP_check_channel __P((int, node_t, channel_t));

void update_recv_seq_X __P((node_t, channel_t));
void register_job      __P((void));
void unregister_job    __P((void));

#endif

#endif

