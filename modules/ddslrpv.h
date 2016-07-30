
/* $Id: ddslrpv.h,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#ifndef _DDSLRPV_H_
#define _DDSLRPV_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

extern boolean_t slrpv_cansend __P((node_t,
				    channel_t));

extern int slrpv_send __P((node_t,
			   channel_t,
			   caddr_t,
			   size_t,
			   void (*) __P((caddr_t, int)),
			   caddr_t,
			   struct proc *));

#ifdef WITH_STABS_PCIBUS_SET
extern int slrpv_send_prot __P((node_t,
				channel_t,
				caddr_t,
				size_t,
				void (*) __P((caddr_t, int)),
				caddr_t,
				struct proc *));
#endif

extern int slrpv_send_piggy_back __P((node_t,
				     channel_t,
       				     caddr_t,
				     size_t,
				     caddr_t,
				     size_t,
				     void (*) __P((caddr_t, int)),
				     caddr_t,
				     struct proc *));

extern int slrpv_send_piggy_back_phys __P((node_t,
					   channel_t,
					   pagedescr_t *,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*) __P((caddr_t, int)),
					   caddr_t,
					   struct proc *));

#ifdef WITH_STABS_PCIBUS_SET
extern int slrpv_send_piggy_back_prot __P((node_t,
					   channel_t,
					   caddr_t,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*) __P((caddr_t, int)),
					   caddr_t,
					   struct proc *));

extern int slrpv_send_piggy_back_prot_phys __P((node_t,
						channel_t,
						pagedescr_t *,
						size_t,
						caddr_t,
						size_t,
						void (*) __P((caddr_t, int)),
						caddr_t,
						struct proc *));
#endif

extern int slrpv_recv __P((node_t,
			   channel_t,
			   caddr_t,
			   size_t,
			   void (*) __P((caddr_t)),
			   caddr_t,
			   struct proc *));

#ifdef WITH_STABS_PCIBUS_SET
extern int slrpv_recv_prot __P((node_t,
				channel_t,
				caddr_t,
				size_t,
				void (*) __P((caddr_t)),
				caddr_t,
				struct proc *));
#endif

extern int slrpv_recv_piggy_back __P((node_t,
			              channel_t,
				      caddr_t,
				      size_t,
				      caddr_t,
				      size_t,
				      void (*) __P((caddr_t)),
				      caddr_t,
				      struct proc *));

extern int slrpv_recv_piggy_back_phys __P((node_t,
					   channel_t,
					   pagedescr_t *,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*) __P((caddr_t)),
					   caddr_t,
					   struct proc *));

#ifdef WITH_STABS_PCIBUS_SET
extern int slrpv_recv_piggy_back_prot __P((node_t,
					   channel_t,
					   caddr_t,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*) __P((caddr_t)),
					   caddr_t,
					   struct proc *));

extern int slrpv_recv_piggy_back_prot_phys __P((node_t,
						channel_t,
						pagedescr_t *,
						size_t,
						caddr_t,
						size_t,
						void (*) __P((caddr_t)),
						caddr_t,
						struct proc *));
#endif

extern void slrpv_reset_channel __P((node_t,
				     channel_t));

extern int slrpv_mlock __P((vm_offset_t,
			    vm_size_t,
			    struct proc *));

extern int slrpv_munlock __P((vm_offset_t,
			      vm_size_t,
			      struct proc *));

#ifdef WITH_STABS_PCIBUS_SET

extern int slrpv_prot_init __P((void));
extern int slrpv_prot_end  __P((void));

extern int slrpv_prot_wire __P((vm_offset_t,
				vm_size_t,
				prot_range_t *,
				int *,
				struct proc *));

extern void slrpv_prot_dump __P((void));

extern void slrpv_prot_garbage_collection __P((void));

#endif

extern int slrpv_set_last_callback __P((node_t,
					channel_t,
					boolean_t (*) __P((caddr_t)),
					caddr_t,
					struct proc *));

extern int slrpv_open_channel      __P((node_t,
					channel_t,
					appclassname_t,
					struct proc *));
extern int slrpv_shutdown0_channel __P((node_t,
				        channel_t,
					seq_t *,
					seq_t *,
					struct proc *));
extern int slrpv_shutdown1_channel __P((node_t,
				        channel_t,
					seq_t,
					seq_t,
					struct proc *));
extern int slrpv_close_channel     __P((node_t,
					channel_t,
					struct proc *));

#endif
