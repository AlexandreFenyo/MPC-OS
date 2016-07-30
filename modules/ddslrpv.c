
/* $Id: ddslrpv.c,v 1.4 2000/02/08 18:54:32 alex Exp $ */

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "put.h"
#include "ddslrpp.h"
#include "ddslrpv.h"
#include "data.h"

#include <vm/vm.h>
#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <vm/lock.h>
#endif
#include <vm/pmap.h>
#include <machine/pmap.h>
#include <vm/vm_map.h>
#include <sys/systm.h>
#include <sys/syslog.h>
#include <vm/vm_pageout.h>
#include <sys/vmmeter.h>

#include <vm/vm_extern.h>
#include <vm/vm_object.h>
#include <vm/vm_prot.h>
#include <vm/default_pager.h>
#include <vm/vm_page.h>
#include <sys/malloc.h>

/************************************************************/
/* Internal MACH macros that we need to access              */
/************************************************************/

#ifdef WITH_STABS_PCIBUS_SET

struct vmspace *vms = NULL;

/*
 * The following MACH internal functions of the kernel VM are labeled static.
 * We need to link to them, thus we extract them from the running kernel
 * and build a weak symbol table (modules/pcibus.S.m4) at module load time.
 */

extern _vm_map_clip_start __P((vm_map_t, vm_map_entry_t, vm_offset_t));
extern _vm_map_clip_end   __P((vm_map_t, vm_map_entry_t, vm_offset_t));

/*
 * Asserts that the starting and ending region
 * addresses fall within the valid range of the map.
 */

#define	VM_MAP_RANGE_CHECK(map, start, end)		\
		{					\
		if (start < vm_map_min(map))		\
			start = vm_map_min(map);	\
		if (end > vm_map_max(map))		\
			end = vm_map_max(map);		\
		if (start > end)			\
			start = end;			\
		}

/*
 * Asserts that the given entry begins at or after
 * the specified address; if necessary,
 * it splits the entry into two.
 */

#ifndef vm_map_clip_start
#define vm_map_clip_start(map, entry, startaddr) \
{ \
	if (startaddr > entry->start) \
		_vm_map_clip_start(map, entry, startaddr); \
}
#endif

/*
 * Asserts that the given entry ends at or before
 * the specified address; if necessary,
 * it splits the entry into two.
 */

#ifndef vm_map_clip_end
#define vm_map_clip_end(map, entry, endaddr) \
{ \
	if (endaddr < entry->end) \
		_vm_map_clip_end(map, entry, endaddr); \
}
#endif

#endif

/*
 ************************************************************
 * slrpv_reset_channel(node, channel) : reset the sequence numbers
 * on a channel with a node.
 * node    : remote node.
 * channel : channel identifier.
 ************************************************************
 */

void
slrpv_reset_channel(node, channel)
                    channel_t channel;
		    node_t    node;
{
  slrpp_reset_channel(node, channel);
}


/*
 ************************************************************
 * fill_pagestab(pagestab, pages, size, prot_write) : find the physical pages
 * composing an area of virtual memory.
 * pagestab   : array containing the result.
 * pages      : virtual address of the area.
 * size       : size of the area.
 * prot_write : indicates that write access should be checked.
 *
 * return values :  0 : array too small.
 *                 -1 : no mapping for some pages.
 *                 -2 : not enough rights.
 *                 >0 : number of entries put in the array on success.
 ************************************************************
 */

static int
fill_pagestab(pagestab, pages, size, prot_write)
              pagedescr_t *pagestab;
              caddr_t pages;
              int size;
	      boolean_t prot_write;
{
  vm_offset_t current_ptr, current_ptr_phys;
  size_t current_offset, forward;
  int pagestab_index;

  TRY("fill_pagestab")

  current_ptr = (vm_offset_t) pages;
  current_offset = 0;
  pagestab_index = 0;

  while (current_offset < size) {
    /* Compute the number of bytes until the end
       of this memory page */
    forward = MIN(PAGE_SIZE - (current_ptr & PAGE_MASK),
		  size - current_offset);

    /* The current page is just after the previous one ? */
    current_ptr_phys = vtophys(current_ptr);

    /* Check that the address is correctly mapped */
    if (!current_ptr_phys)
      return -1;

    /*
     * Check that the user has sufficient rights to access data at this address.
     * Note that we directly scan the MMU tables instead of accessing vm pages
     * objects maintained by MACH, to get better performances.
     */
    if (prot_write && !(((int) (*vtopte(current_ptr))) & PG_RW))
      return -2;

    if (pagestab_index > 0 &&
	(vm_offset_t) pagestab[pagestab_index - 1].addr +
	pagestab[pagestab_index - 1].size ==
	current_ptr_phys &&
	/* Check that the previous area is not too big for PCI-DDC ?
	(PCI-DDC is limited to 65535 bytes per page) */
	pagestab[pagestab_index - 1].size + forward <
	0x10000UL)
      /* OK : we aggregate the current area with
	 the previous one */
      pagestab[pagestab_index - 1].size += forward;

    else {
      /* No : either we are at the limit of 2 contig. areas,
         or we must start a new area because the previous one
	 is very big */

      if (pagestab_index >= PAGESTAB_SIZE) {
	log(LOG_ERR,
	    "fill_pagestab(): pagestab full\n");
	return 0;
      }
      pagestab[pagestab_index].addr = (caddr_t) current_ptr_phys;
      pagestab[pagestab_index].size = forward;
      pagestab_index++;

    }

    current_offset = current_offset + forward;
    current_ptr = current_ptr + forward;
  }

#ifdef DEBUG_HSL
  {
    int i;

    for (i = 0;
	 i < pagestab_index;
	 i++)
      log(LOG_DEBUG,
	  "pagestab[%d] = {0x%x, 0x%x}\n",
	  i,
	  (int) pagestab[i].addr,
	  pagestab[i].size);
  }
#endif

  return pagestab_index;
}              


/*
 ************************************************************
 * slrpv_cansend(dest, channel) :
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
slrpv_cansend(dest, channel)
              node_t dest;
	      channel_t channel;
{
  return slrpp_cansend(dest, channel);
}


/*
 ************************************************************
 * slrpv_send(dest, channel, pages, size, fct, param, proc) :
 * try to send data to a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * pages   : virtual address of the beginning of the data.
 * size    : size of data.
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
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized or not enough
 *                               structures allocated at compilation
 *                               time to handle this board.
 *                 EINVAL      : size is NULL.
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 EFAULT      : should not occur, unrecoverable error,
 *                               contact the author if it happens.
 *                 ENOENT      : this board is not initialized.
 *                 EIO         : no mapping for some pages.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_send(dest, channel, pages, size, fct, param, proc)
           node_t dest;
           channel_t channel;
           caddr_t pages;
           size_t size;
           void (*fct) __P((caddr_t, int));
           caddr_t param;
           struct proc *proc;
{
  int nb_pages;
  static pagedescr_t pagestab[PAGESTAB_SIZE];

  TRY("slrpv_send")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   FALSE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    /* Should not happen */
    return EACCES;

  return slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages,
		    fct,
		    param,
		    proc);
}


#ifdef WITH_STABS_PCIBUS_SET

/*
 ************************************************************
 * slrpv_send_prot(...) : same as slrpv_send(...), also protect the memory.
 ************************************************************
 */

int
slrpv_send_prot(dest, channel, pages, size, fct, param, proc)
                node_t dest;
                channel_t channel;
                caddr_t pages;
                size_t size;
                void (*fct) __P((caddr_t, int));
                caddr_t param;
                struct proc *proc;
{
  int s;
  int nb_pages;
  static pagedescr_t pagestab[PAGESTAB_SIZE];
  pending_send_entry_t *psentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_send_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  /************************************************************/

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - START prot_wire");
#endif

#ifdef MPC_PERFMON
  __asm__("cli");
  PERFMON_START;
#endif

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages,
			size,
			prot_tab,
			&len,
			proc);

#ifdef MPC_PERFMON
  PERFMON_STOP("prot_wire");
  __asm__("sti");
#endif

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - END prot_wire");
#endif

  if (ret) {
    log(LOG_ERR,
	"slrpv_send_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - START fill_pagestab");
#endif

#ifdef MPC_PERFMON
  __asm__("cli");
  PERFMON_START;
#endif

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   FALSE);

#ifdef MPC_PERFMON
  PERFMON_STOP("fill_pagestab");
  __asm__("sti");
#endif

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - END fill_pagestab");
  {
    char ch[80];
    sprintf(ch, "nb_pages = %d", nb_pages);
    add_event_string(ch);
  }
#endif

  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    /* Should not happen */
    return EACCES;

  /************************************************************/

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - START _slrpp_send");
#endif

  ret = _slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages,
		    fct,
		    param,
		    &psentry,
		    proc,
		    FALSE);

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - END _slrpp_send");
#endif

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	psentry->prot_map_ref_tab,
	sizeof(psentry->prot_map_ref_tab));

  s = splhigh();
  psentry->post_treatment_lock = 0;
  splx(s);

  if (!len)
    return ENOMEM;

#ifdef MPC_PERFMON
  add_event_string("slrpv_send_prot - END");
#endif

  return 0;
}

#endif


/*
 ************************************************************
 * slrpv_send_piggy_back(dest, channel, pages, size, pages2, size2,
 * fct, param, proc) :
 * try to send data to a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * pages   : virtual address of the beginning of the first set of data.
 * size    : size of data.
 * pages2  : virtual address of the beginning of the second set of data.
 * size2   : size of data.
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
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized or not enough
 *                               structures allocated at compilation
 *                               time to handle this board.
 *                 EINVAL      : size is NULL.
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 EFAULT      : should not occur, unrecoverable error,
 *                               contact the author if it happens.
 *                 ENOENT      : this board is not initialized.
 *                 EIO         : no mapping for some pages.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_send_piggy_back(dest, channel, pages, size, pages2, size2,
		      fct, param, proc)
                      node_t dest;
                      channel_t channel;
                      caddr_t pages, pages2;
                      size_t size, size2;
                      void (*fct) __P((caddr_t, int));
                      caddr_t param;
                      struct proc *proc;
{
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];

  TRY("slrpv_send_piggy_back")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   FALSE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    /* Should not happen */
    return EACCES;

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    FALSE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    /* Should not happen */
    return EACCES;

  return slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    proc);
}


/*
 ************************************************************
 * slrpv_send_piggy_back_phys(dest, channel, ptab, size, pages2, size2,
 * fct, param, proc) :
 * try to send data to a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * ptab    : table of physical areas.
 * size    : size of the table.
 * pages2  : virtual address of the beginning of the second set of data.
 * size2   : size of data.
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
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized or not enough
 *                               structures allocated at compilation
 *                               time to handle this board.
 *                 EINVAL      : size is NULL.
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 EFAULT      : should not occur, unrecoverable error,
 *                               contact the author if it happens.
 *                 ENOENT      : this board is not initialized.
 *                 EIO         : no mapping for some pages.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_send_piggy_back_phys(dest, channel, ptab, size, pages2, size2,
			   fct, param, proc)
                           node_t dest;
			   channel_t channel;
			   pagedescr_t *ptab;
			   caddr_t pages2;
			   size_t size, size2;
			   void (*fct) __P((caddr_t, int));
			   caddr_t param;
			   struct proc *proc;
{
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];

  TRY("slrpv_send_piggy_back")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  if (!size || size >= PAGESTAB_SIZE) return E2BIG;
  for (nb_pages = 0; nb_pages < size; nb_pages++)
    pagestab[nb_pages] = ptab[nb_pages];

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    FALSE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    /* Should not happen */
    return EACCES;

  return slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    proc);
}

#ifdef WITH_STABS_PCIBUS_SET

/*
  ************************************************************
  * slrpv_send_piggy_back_prot(...) : same as slrpv_send_piggy_back(...),
  * also protect the memory.
  *
  * The piggy backed data are not protected.
  ************************************************************
  */

int
slrpv_send_piggy_back_prot(dest, channel, pages, size, pages2, size2,
			   fct, param, proc)
                           node_t dest;
			   channel_t channel;
			   caddr_t pages, pages2;
			   size_t size, size2;
			   void (*fct) __P((caddr_t, int));
			   caddr_t param;
			   struct proc *proc;
{
  int s;
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];
  pending_send_entry_t *psentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_send_piggy_back_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  /************************************************************/

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages2,
			size2,
			prot_tab,
			&len,
			proc);

  if (ret) {
    log(LOG_ERR,
	"slrpv_send_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   FALSE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    /* Should not happen */
    return EACCES;

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    FALSE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    /* Should not happen */
    return EACCES;

  /************************************************************/

  ret = _slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    &psentry,
		    proc,
		    FALSE);

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	psentry->prot_map_ref_tab,
	sizeof(psentry->prot_map_ref_tab));

  s = splhigh();
  psentry->post_treatment_lock = 0;
  splx(s);

  if (!len)
    return ENOMEM;

  return 0;
}


/*
  ************************************************************
  * slrpv_send_piggy_back_prot_phys(...) : same as slrpv_send_piggy_back_phys(...),
  * also protect the memory.
  *
  * The piggy backed data are not protected.
  ************************************************************
  */

int
slrpv_send_piggy_back_prot_phys(dest, channel, ptab, size, pages2, size2,
				fct, param, proc)
                                node_t dest;
				channel_t channel;
				pagedescr_t *ptab;
				caddr_t pages2;
				size_t size, size2;
				void (*fct) __P((caddr_t, int));
				caddr_t param;
				struct proc *proc;
{
  int s;
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];
  pending_send_entry_t *psentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_send_piggy_back_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_send++;
    splx(s);
  }
#endif

  /************************************************************/

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages2,
			size2,
			prot_tab,
			&len,
			proc);

  if (ret) {
    log(LOG_ERR,
	"slrpv_send_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

  if (!size || size >= PAGESTAB_SIZE) return E2BIG;
  for (nb_pages = 0; nb_pages < size; nb_pages++)
    pagestab[nb_pages] = ptab[nb_pages];

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    FALSE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    /* Should not happen */
    return EACCES;

  /************************************************************/

  ret = _slrpp_send(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    &psentry,
		    proc,
		    FALSE);

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	psentry->prot_map_ref_tab,
	sizeof(psentry->prot_map_ref_tab));

  s = splhigh();
  psentry->post_treatment_lock = 0;
  splx(s);

  if (!len)
    return ENOMEM;

  return 0;
}

#endif


/*
 ************************************************************
 * slrpv_recv(dest, channel, pages, size, fct, param, proc) :
 * ask to receive data from a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * pages   : virtual address of the beginning of the area ready to receive
 *           data.
 * size    : size of the area.
 * fct     : callback function called when data have been correctly
 *           received.
 * param   : parameter provided to the callback function when called.
 * proc    : proc structure of the calling process.
 *
 * return values : 0           : success.
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 ENXIO       : not enough structures allocated at
 *                             :  compilation time to handle this board.
 *                 ENOENT      : this board is not initialized.
 *                 EIO         : no mapping for some pages.
 *                 EACCES      : not enough rights.
 *                 ENOMEM      : size too big.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_recv(dest, channel, pages, size, fct, param, proc)
           node_t dest;
           channel_t channel;
           caddr_t pages;
           size_t size;
           void (*fct) __P((caddr_t));
           caddr_t param;
           struct proc *proc;
{
  int nb_pages;
  static pagedescr_t pagestab[PAGESTAB_SIZE];

  TRY("slrpv_recv")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   TRUE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    return EACCES;

  return slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages,
		    fct,
		    param,
		    proc);
}


#ifdef WITH_STABS_PCIBUS_SET

/*
 ************************************************************
 * slrpv_recv_prot(...) : same as slrpv_recv(...), also protect the memory.
 ************************************************************
 */

int
slrpv_recv_prot(dest, channel, pages, size, fct, param, proc)
                node_t dest;
                channel_t channel;
                caddr_t pages;
                size_t size;
                void (*fct) __P((caddr_t));
                caddr_t param;
                struct proc *proc;
{
  int s;
  int nb_pages;
  static pagedescr_t pagestab[PAGESTAB_SIZE];
  pending_receive_entry_t *prentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_recv_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  /************************************************************/

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages,
			size,
			prot_tab,
			&len,
			proc);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   TRUE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    return EACCES;

  /************************************************************/

  ret = _slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages,
		    fct,
		    param,
		    &prentry,
		    proc,
		    FALSE);

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	prentry->prot_map_ref_tab,
	sizeof(prentry->prot_map_ref_tab));

  s = splhigh();
  prentry->post_treatment_lock = 0;
  splx(s);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  if (!len)
    return ENOMEM;

  return 0;
}

#endif


/*
 ************************************************************
 * slrpv_recv_piggy_back(dest, channel, pages, size, pages2, size2,
 * fct, param, proc) :
 * ask to receive data from a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * pages   : virtual address of the beginning of the first area ready
 *           to receive data.
 * size    : size of the area.
 * pages   : virtual address of the beginning of the second area ready
 *           to receive data.
 * size    : size of the area.
 * fct     : callback function called when data have been correctly
 *           received.
 * param   : parameter provided to the callback function when called.
 * proc    : proc structure of the calling process.
 *
 * return values :          0  : success.
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 ENXIO       : not enough structures allocated at
 *                             :  compilation time to handle this board.
 *                 ENOENT      : this board is not initialized.
 *                 EACCES      : not enough rights.
 *                 ENOMEM      : size too big.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_recv_piggy_back(dest, channel, pages, size, pages2, size2,
		      fct, param, proc)
                      node_t dest;
                      channel_t channel;
                      caddr_t pages, pages2;
                      size_t size, size2;
                      void (*fct) __P((caddr_t));
                      caddr_t param;
                      struct proc *proc;
{
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];

  TRY("slrpv_recv_piggy_back")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   TRUE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    return EACCES;

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    TRUE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    return EACCES;

  return slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    proc);
}

/*
 ************************************************************
 * slrpv_recv_piggy_back_phys(dest, channel, ptab, size, pages2, size2,
 * fct, param, proc) :
 * ask to receive data from a remote node on a specific channel.
 * dest    : remote node.
 * channel : channel identifier.
 * ptab    : table of physical areas.
 * size    : size of the table.
 * pages   : virtual address of the beginning of the second area ready
 *           to receive data.
 * size    : size of the area.
 * fct     : callback function called when data have been correctly
 *           received.
 * param   : parameter provided to the callback function when called.
 * proc    : proc structure of the calling process.
 *
 * return values :          0  : success.
 *                 E2BIG       : an internal array used to handle
 *                               a description of the physical pages
 *                               composing the area the user wants to send
 *                               is too small (the area is made of
 *                               too many uncontig. groups of pages).
 *                 ENXIO       : SLR/P not initialized.
 *                 EINVAL      : size is NULL or we're the remote node
 *                               (local loop not allowed).
 *                 EWOULDBLOCK : tsleep() returned EWOULDBLOCK.
 *                 EINTR       : tsleep() returned EINTR.
 *                 ERESTART    : tsleep() returned ERESTART.
 *                 ENXIO       : not enough structures allocated at
 *                             :  compilation time to handle this board.
 *                 ENOENT      : this board is not initialized.
 *                 EACCES      : not enough rights.
 *                 ENOMEM      : size too big.
 *                 ENOTCONN    : channel closed.
 *                 ESHUTDOWN   : channel is being closed.
 ************************************************************
 */

int
slrpv_recv_piggy_back_phys(dest, channel, ptab, size, pages2, size2,
			   fct, param, proc)
                           node_t dest;
                           channel_t channel;
		           pagedescr_t *ptab;
                           caddr_t pages2;
                           size_t size, size2;
                           void (*fct) __P((caddr_t));
                           caddr_t param;
			   struct proc *proc;
{
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];

  TRY("slrpv_recv_piggy_back")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  if (!size || size >= PAGESTAB_SIZE) return E2BIG;
  for (nb_pages = 0; nb_pages < size; nb_pages++)
    pagestab[nb_pages] = ptab[nb_pages];

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    TRUE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    return EACCES;

  return slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    proc);
}

#ifdef WITH_STABS_PCIBUS_SET

/*
 ************************************************************
 * slrpv_recv_piggy_back_prot(...) : same as slrpv_recv_piggy_back(...),
 * also protect the memory.
 *
 * The piggy backed data are not protected.
 ************************************************************
 */

int
slrpv_recv_piggy_back_prot(dest, channel, pages, size, pages2, size2,
		           fct, param, proc)
                           node_t dest;
			   channel_t channel;
			   caddr_t pages, pages2;
			   size_t size, size2;
			   void (*fct) __P((caddr_t));
			   caddr_t param;
			   struct proc *proc;
{
  int s;
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];
  pending_receive_entry_t *prentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_recv_piggy_back_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  /************************************************************/

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages2,
			size2,
			prot_tab,
			&len,
			proc);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

  nb_pages = fill_pagestab(pagestab,
			   pages,
			   size,
			   TRUE);
  if (!nb_pages)
    return E2BIG;
  if (nb_pages == -1)
    return EIO;
  if (nb_pages == -2)
    return EACCES;

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    TRUE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    return EACCES;

  /************************************************************/

  ret = _slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    &prentry,
		    proc,
		    FALSE);

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	prentry->prot_map_ref_tab,
	sizeof(prentry->prot_map_ref_tab));

  s = splhigh();
  prentry->post_treatment_lock = 0;
  splx(s);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  if (!len)
    return ENOMEM;

  return 0;
}


/*
 ************************************************************
 * slrpv_recv_piggy_back_prot_phys(...) : same as slrpv_recv_piggy_back_phys(...),
 * also protect the memory.
 *
 * The piggy backed data are not protected.
 ************************************************************
 */

int
slrpv_recv_piggy_back_prot_phys(dest, channel, ptab, size, pages2, size2,
				fct, param, proc)
                                node_t dest;
				channel_t channel;
				pagedescr_t *ptab;
				caddr_t pages2;
				size_t size, size2;
				void (*fct) __P((caddr_t));
				caddr_t param;
				struct proc *proc;
{
  int s;
  int nb_pages, nb_pages2;
  static pagedescr_t pagestab[2 * PAGESTAB_SIZE];
  pending_receive_entry_t *prentry = NULL;
  int len;
  int ret;
  prot_range_t prot_tab[PROT_MAP_REF_TAB_SIZE];

  TRY("slrpv_recv_piggy_back_prot")

#ifdef MPC_STATS
  {
    int s;

    s = splhigh();
    driver_stats.calls_slrpv_recv++;
    splx(s);
  }
#endif

  /************************************************************/

  bzero(prot_tab, sizeof prot_tab);
  len = PROT_MAP_REF_TAB_SIZE;
  ret = slrpv_prot_wire((vm_offset_t) pages2,
			size2,
			prot_tab,
			&len,
			proc);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  /************************************************************/

  if (!size || size >= PAGESTAB_SIZE) return E2BIG;
  for (nb_pages = 0; nb_pages < size; nb_pages++)
    pagestab[nb_pages] = ptab[nb_pages];

  nb_pages2 = fill_pagestab(pagestab + nb_pages,
			    pages2,
			    size2,
			    TRUE);

  if (!nb_pages2 ||
      nb_pages + nb_pages2 > PAGESTAB_SIZE)
    return E2BIG;
  if (nb_pages2 == -1)
    return EIO;
  if (nb_pages2 == -2)
    return EACCES;

  /************************************************************/

  ret = _slrpp_recv(dest,
		    channel,
		    pagestab,
		    nb_pages + nb_pages2,
		    fct,
		    param,
		    &prentry,
		    proc,
		    FALSE);

  if (ret)
    return ret;

  /* The pending send entry is protected by post_treatment_lock : even
     if the SEND is completed now, the entry cannot be deleted */

  bcopy(prot_tab,
	prentry->prot_map_ref_tab,
	sizeof(prentry->prot_map_ref_tab));

  s = splhigh();
  prentry->post_treatment_lock = 0;
  splx(s);

  if (ret) {
    log(LOG_ERR,
	"slrpv_recv_piggy_back_prot(): protection failed (%d)\n",
	ret);
    return ENOMEM;
  }

  if (!len)
    return ENOMEM;

  return 0;
}

#endif


/*
 ************************************************************
 * slrpv_mlock(lock_addr, lock_len, proc) : locks into memory
 * the physical pages associated with the virtual address range
 * starting at lock_addr for lock_len bytes.
 * lock_addr : beginning of the range.
 * lock_len  : length of the range.
 * proc      : proc structure of the calling process.
 *
 * Note that the lock_addr parameter should be aligned to a
 * multiple of the page size.
 *
 * return values : 0      : success.
 *                 EINVAL : The address given is not page aligned
 *                          or the length is negative.
 *                 EAGAIN : Locking the indicated range would
 *                          exceed either the system or
 *                          per-process limit for locked memory.
 *                 ENOMEM : Some portion of the indicated address
 *                          range is not allocated.
 *                          There was an error faulting/mapping
 *                          a page.
 ************************************************************
 */

#ifndef _SMP_SUPPORT
int
slrpv_mlock(lock_addr, lock_len, proc)
            vm_offset_t lock_addr;
            vm_size_t   lock_len;
            struct proc *proc;
{
  vm_offset_t addr;
  vm_size_t size, pageoff;
  int error;

  TRY("slrpv_mlock")

  addr = (vm_offset_t) lock_addr;
  size = lock_len;

  pageoff = (addr & PAGE_MASK);
  addr -= pageoff;
  size += pageoff;
  size = (vm_size_t) round_page(size);

  /* disable wrap around */
  if (addr + size < addr)
    return EINVAL;

  /*
  if (atop(size) + cnt.v_wire_count > vm_page_max_wired)
    return EAGAIN;

#ifdef pmap_wired_count
  if (size + ptoa(pmap_wired_count(vm_map_pmap(&proc->p_vmspace->vm_map))) >
      proc->p_rlimit[RLIMIT_MEMLOCK].rlim_cur)
    return EAGAIN;
#else
  error = suser(proc->p_ucred,
		&proc->p_acflag);
  if (error)
    return error;
#endif
  */

  error = vm_map_pageable(&proc->p_vmspace->vm_map,
			  addr,
			  addr + size,
			  FALSE);

  return error == KERN_SUCCESS ? 0 : ENOMEM;
}

#else

int
slrpv_mlock(lock_addr, lock_len, proc)
            vm_offset_t lock_addr;
            vm_size_t   lock_len;
            struct proc *proc;
{
	vm_offset_t addr;
	vm_size_t size, pageoff;
	int error;

	addr = (vm_offset_t) lock_addr;
	size = lock_len;

	pageoff = (addr & PAGE_MASK);
	addr -= pageoff;
	size += pageoff;
	size = (vm_size_t) round_page(size);

	/* disable wrap around */
	if (addr + size < addr)
		return (EINVAL);

	if (atop(size) + cnt.v_wire_count > vm_page_max_wired)
		return (EAGAIN);

#if 0
#ifdef pmap_wired_count
	if (size + ptoa(pmap_wired_count(vm_map_pmap(&p->p_vmspace->vm_map))) >
	    proc->p_rlimit[RLIMIT_MEMLOCK].rlim_cur)
		return (ENOMEM);
#else
	error = suser(proc->p_ucred, &proc->p_acflag);
	if (error)
		return (error);
#endif
#endif

	error = vm_map_user_pageable(&proc->p_vmspace->vm_map, addr, addr + size, FALSE);
	return (error == KERN_SUCCESS ? 0 : ENOMEM);
}
#endif

/*
 ************************************************************
 * slrpv_munlock(lock_addr, lock_len, proc) : unlocks into memory
 * the physical pages associated with the virtual address range
 * starting at lock_addr for lock_len bytes.
 * lock_addr : beginning of the range.
 * lock_len  : length of the range.
 * proc      : proc structure of the calling process.
 *
 * Note that the lock_addr parameter should be aligned to a
 * multiple of the page size.
 *
 * return values : 0      : success.
 *                 EINVAL : The address given is not page aligned
 *                          or the length is negative.
 *                 ENOMEM : Some portion of the indicated address
 *                          range is not allocated.
 *                          Some portion of the indicated address
 *                          range is not locked.
 ************************************************************
 */

#ifndef _SMP_SUPPORT
int
slrpv_munlock(lock_addr, lock_len, proc)
              vm_offset_t lock_addr;
              vm_size_t   lock_len;
              struct proc *proc;
{
  vm_offset_t addr;
  vm_size_t size;
  int error;

  addr = (vm_offset_t) lock_addr;
  if ((addr & PAGE_MASK) ||
      lock_addr + lock_len < lock_addr)
    return EINVAL;

  /*
#ifndef pmap_wired_count
  error = suser(proc->p_ucred,
		&proc->p_acflag);
  if (error)
    return error;
#endif
  */
  size = round_page((vm_size_t) lock_len);

  error = vm_map_pageable(&proc->p_vmspace->vm_map,
			  addr,
			  addr + size,
			  TRUE);

  return error == KERN_SUCCESS ? 0 : ENOMEM;
}

#else

int
slrpv_munlock(lock_addr, lock_len, proc)
              vm_offset_t lock_addr;
              vm_size_t   lock_len;
              struct proc *proc;
{
	vm_offset_t addr;
	vm_size_t size, pageoff;
	int error;

	addr = (vm_offset_t) lock_addr;
	size = lock_len;

	pageoff = (addr & PAGE_MASK);
	addr -= pageoff;
	size += pageoff;
	size = (vm_size_t) round_page(size);

	/* disable wrap around */
	if (addr + size < addr)
		return (EINVAL);

#if 0
#ifndef pmap_wired_count
	error = suser(proc->p_ucred, &proc->p_acflag);
	if (error)
		return (error);
#endif
#endif

	error = vm_map_user_pageable(&proc->p_vmspace->vm_map, addr, addr + size, TRUE);
	return (error == KERN_SUCCESS ? 0 : ENOMEM);
}
#endif

#ifdef WITH_STABS_PCIBUS_SET

/*
 ************************************************************
 * slrpv_prot_init() : initialize the memory protection map,
 * including a vm_map and pmap.
 *
 * return values : 0     : success.
 *                 EBUSY : already initialized.
 ************************************************************
 */

int
slrpv_prot_init(void)
{
  if (vms) {
    log(LOG_ERR,
	"SLRP/V already initialized\n");
    return EBUSY;
  }

#ifndef _SMP_SUPPORT
  vms = vmspace_alloc(round_page(VM_MIN_ADDRESS),
		      trunc_page(VM_MAXUSER_ADDRESS),
		      FALSE); /* FALSE (not pageable) really needed ??? */
#else
  vms = vmspace_alloc(round_page(VM_MIN_ADDRESS),
		      trunc_page(VM_MAXUSER_ADDRESS));
#endif

#ifndef _SMP_SUPPORT
  /* No process associated with this vm_space */
  vms->vm_upages_obj = NULL;
#endif

  return 0;
}


/*
 ************************************************************
 * slr_vmspace_free(vm) : free a vmspace.
 *
 * This function is a replacement for the MACH vmspace_free() function.
 * The FreeBSD version cannot handle a vm space without upages !
 ************************************************************
 */

static void
slr_vmspace_free(vm)
	         register struct vmspace *vm;
{
#ifndef _SMP_SUPPORT

  if (vm->vm_refcnt == 0)
    panic("vmspace_free: attempt to free already freed vmspace");

  if (--vm->vm_refcnt == 0) {
    /*
     * Lock the map, to wait out all other references to it.
     * Delete all of the mappings and pages they hold, then call
     * the pmap module to reclaim anything left.
     */
    vm_map_lock(&vm->vm_map);
    vm_map_delete(&vm->vm_map,
		  vm->vm_map.min_offset,
		  vm->vm_map.max_offset);
    vm_map_unlock(&vm->vm_map);

    while( vm->vm_map.ref_count != 1) {
#ifdef MPC_PERFMON
      perfmon_invalid = 1;
#endif
      tsleep(&vm->vm_map.ref_count, PVM, "vmsfre", 0);
    }
    --vm->vm_map.ref_count;

    pmap_release(&vm->vm_pmap);
    FREE(vm, M_VMMAP);
  } else {
    wakeup(&vm->vm_map.ref_count);
  }

#else

  vmspace_free(vm);

#endif
}


/*
 ************************************************************
 * slrpv_prot_end() : destroy the memory protection map.
 * This will free all the unreferenced wired vm objects.
 *
 * return values : 0      : success.
 *                 ENOENT : not initialized.
 ************************************************************
 */

int
slrpv_prot_end(void)
{
  if (!vms) {
    log(LOG_ERR,
	"SLRP/V not initialized\n");
    return ENOENT;
  }

  slr_vmspace_free(vms);
  vms = NULL;

  return 0;
}


/*
 ************************************************************
 * slrpv_prot_dump() : dump the protection map.
 ************************************************************
 */

void
slrpv_prot_dump(void)
{
  vm_map_entry_t vms_entry;

  if (!vms) {
    log(LOG_ERR,
	"SLRP/V not initialized\n");
    return;
  }

  log(LOG_ERR,
      "DUMPING THE PROTECTION MAP\n");

  vms_entry = vms->vm_map.header.next;

  while (vms_entry != &vms->vm_map.header) {
    log(LOG_INFO,
	"vm_map_entry(0x%x): start(0x%x) size(0x%x) offs(0x%x) obj(0x%x size=0x%x)\n",
	(int) vms_entry,
	(int) vms_entry->start,
	(int) vms_entry->end - vms_entry->start,
	(int) vms_entry->offset,
	(int) vms_entry->object.vm_object,
	(int) IDX_TO_OFF(vms_entry->object.vm_object->size));
    vms_entry = vms_entry->next;
  }
}


/*
 ************************************************************
 * slrpv_prot_ref_object(obj, ref_tab_p, ref_len) : reference an object
 * into the protection map.
 * obj       : vm object to insert in the protection map.
 * ref_tab_p : table to fill with the new range allocated
 *             in the protection map.
 * ref_len   : table length.
 ************************************************************
 */

static void
slrpv_prot_ref_object(obj, ref_tab_p, ref_len)
                      struct vm_object *obj;
		      prot_range_t **ref_tab_p;
		      int *ref_len;
{
  vm_offset_t addr;
  vm_ooffset_t max_ooffset = 0;
  int ret;

  vm_map_entry_t vms_entry;

  if (!vms) {
    log(LOG_ERR,
	"SLRP/V not initialized\n");
    return;
  }

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "slrpv_prot_ref_object(): object=0x%x objsize=0x%x\n",
      (int) obj,
      (int) IDX_TO_OFF(obj->size));
#endif

  vm_map_lock(&vms->vm_map);

  /* Does the object be already protected ? */

  /* We may use a map hint to get better performances... */
  vms_entry = vms->vm_map.header.next;

  while (vms_entry != &vms->vm_map.header) {

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"slrpv_prot_ref_object(): exam. entry 0x%x (assoc. obj=0x%x, objsize=0x%x)\n",
	(int) vms_entry,
	(int) vms_entry->object.vm_object,
	(int) IDX_TO_OFF(vms_entry->object.vm_object->size));

    log(LOG_DEBUG,
	"                         entry start=0x%x size=0x%x\n",
	vms_entry->start,
	vms_entry->end - vms_entry->start);
#endif

    if (obj == vms_entry->object.vm_object) {
      /* The object is mapped into the protection map */

      /* Does the object be ENTIRELY protected (by this entry and others) ? */
      if (vms_entry->end - vms_entry->start + vms_entry->offset <
	  IDX_TO_OFF(obj->size)) {
	max_ooffset = MAX(max_ooffset,
			  vms_entry->end - vms_entry->start + vms_entry->offset);

	/* No, continue to see if another entry maps the same object */
	vms_entry = vms_entry->next;
	continue;
      }

      vm_map_unlock(&vms->vm_map);
      return; /* The object is already protected */
    }

    vms_entry = vms_entry->next;
  }

  /* The object is not entirely protected */

  if (IDX_TO_OFF(obj->size) - max_ooffset <= 0)
    /* Should not happen... */
    log(LOG_ERR,
	"slrpv_prot_ref_object: unrecoverable error, invalid object size\n");

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "slrpv_prot_ref_object: max_ooffset = 0x%x\n",
      (int) max_ooffset);
#endif

  /* Find space in the protection map to reference the object */
  /* Don't call vm_map_find() but use vm_map_findspace()/vm_map_insert()
     because vm_map_find() locks the map, and it is already locked here */

  ret = vm_map_findspace(&vms->vm_map,
			 NULL,
			 IDX_TO_OFF(obj->size) - max_ooffset,
			 &addr);
  if (ret) {
    vm_map_unlock(&vms->vm_map);
    log(LOG_ERR,
	"slrpv_prot_ref_object: unrecoverable error, map full !\n");
    return;
  }

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "vm_map_insert(obj=0x%x offs=0x%x start=0x%x size=0x%x prot=0x%x max=0x%x cow=0x%x)\n",
      (int) obj,
      (int) max_ooffset,
      (int) addr,
      (int) (IDX_TO_OFF(obj->size) - max_ooffset),
      (int) VM_PROT_ALL,
      (int) VM_PROT_ALL,
      (int) 0);
#endif

  ret = vm_map_insert(&vms->vm_map,
		      obj,
		      max_ooffset,
		      addr,
		      addr + (IDX_TO_OFF(obj->size) - max_ooffset),
		      VM_PROT_ALL,
		      VM_PROT_ALL,
		      0);

  if (ret != KERN_SUCCESS)
    log(LOG_ERR,
	"slrpv_prot_ref_object: unrecoverable error, insertion failed\n");

  /*
   * Save the entrie(s) that overlap our virtual range of addresses
   * in the newly mapped object.
   */

  if (*ref_tab_p != NULL)
    if (*ref_len > 0) {
      (*ref_tab_p)->start = addr;
      (*ref_tab_p)->size  = IDX_TO_OFF(obj->size) - max_ooffset;
      (*ref_tab_p)++;
      (*ref_len)--;
    } else
      log(LOG_ERR,
	  "slrpv_prot_ref_object(): unrecoverable error, garbage collection corrupted\n");

  /*
   * Assume that an entry in the protection map now references
   * the protected object.
   *
   * Note that until now, the object had a positive reference count
   * because it was referenced by an entry in the locked process
   * vm map. Since we will soon unlock this map, we must
   * increase before the object reference count to be sure
   * the object cannot be destroyed.
   */
  if (ret == KERN_SUCCESS)
    vm_object_reference(obj);

  /* We must unlock the map before changing the pageability, because
     vm_map_pageable() will lock the map */
  vm_map_unlock(&vms->vm_map);

  /* Wire the newly mapped range */
  if (ret == KERN_SUCCESS) {
    ret = vm_map_pageable(&vms->vm_map,
			  addr,
			  addr + (IDX_TO_OFF(obj->size) - max_ooffset),
			  FALSE);

    if (ret != KERN_SUCCESS)
      log(LOG_ERR,
	  "slrpv_prot_ref_object: unrecoverable error, pageability error\n");
  }
}


/*
 ************************************************************
 * slrpv_prot_wire(prot_addr, prot_len, ref_tab, proc) :
 * wire and protect a range of virtual addresses.
 * prot_addr : start address.
 * prot_len  : length.
 * ref_tab   : table of vm map entries to fill.
 * ref_len   : size of ref_tab.
 * proc      : calling process.
 *
 * return values : 0 : success.
 *                 many other standard error codes.
 ************************************************************
 */

#ifndef _SMP_SUPPORT

int
slrpv_prot_wire(prot_addr, prot_len, ref_tab, ref_len, proc)
                vm_offset_t           prot_addr;
                vm_size_t             prot_len;
		prot_range_t         *ref_tab;
		int                  *ref_len;
                struct proc          *proc;
{
  /* SEE vm_map_user_pageable() */

  struct proc *p;

  vm_offset_t addr;
  vm_size_t size, pageoff;
  /* int error; */

  register vm_map_t map;
  register vm_offset_t start;
  register vm_offset_t end;

  register vm_map_entry_t entry;
  vm_map_entry_t start_entry;
  int rv;

  /* ref_tab only acts like a "C" language rvalue. Convert it to a lvalue. */
  prot_range_t *ref_tab_p = ref_tab;

  addr = prot_addr;
  size = prot_len;
  p    = proc;

  pageoff = (addr & PAGE_MASK);
  addr -= pageoff;
  size += pageoff;
  size = (vm_size_t) round_page(size);

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "slrpv_prot_wire(): addr=0x%x size=0x%x\n",
      addr,
      size);
#endif

  /* disable wrap around */
  if (addr + size < addr)
    return EINVAL;

  /*
  if (atop(size) + cnt.v_wire_count > vm_page_max_wired)
    return EAGAIN;

#ifdef pmap_wired_count
  if (size + ptoa(pmap_wired_count(vm_map_pmap(&p->p_vmspace->vm_map))) >
      p->p_rlimit[RLIMIT_MEMLOCK].rlim_cur)
    return EAGAIN;
#else
  error = suser(p->p_ucred, &p->p_acflag);
  if (error)
    return error;
#endif
  */

  map   = &proc->p_vmspace->vm_map;
  start = addr;
  end   = addr + size;

/*
 * Implement the semantics of mlock
 */

  vm_map_lock(map);
  VM_MAP_RANGE_CHECK(map, start, end);

  if (vm_map_lookup_entry(map, start, &start_entry) == FALSE) {
    vm_map_unlock(map);
    return ENOMEM /* KERN_INVALID_ADDRESS */;
  }

  /*
   * Because of the possiblity of blocking, etc.  We restart
   * through the process's map entries from beginning so that
   * we don't end up depending on a map entry that could have
   * changed.
   */
 rescan:

  entry = start_entry;

  while ((entry != &map->header) && (entry->start < end)) {

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"slrpv_prot_wire(): current entry = 0x%x (start=0x%x size=0x%x)\n",
	entry,
	(int) entry->start,
	entry->end - entry->start);
#endif

    if (entry->eflags & MAP_ENTRY_USER_WIRED) {
      /* Entry already wired */

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   entry already wired - obj_size = 0x%x\n",
	  (int) IDX_TO_OFF(entry->object.vm_object->size));
#endif

      /* We must wire the object if this has not already been done */
      slrpv_prot_ref_object(entry->object.vm_object,
			    &ref_tab_p,
			    ref_len);

      entry = entry->next;
      continue;
    }
			
    if (entry->wired_count != 0) {
#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   wired_count != 0\n");
#endif

      entry->wired_count++;
      entry->eflags |= MAP_ENTRY_USER_WIRED;

      /* We must wire the object if this has not already been done */
      slrpv_prot_ref_object(entry->object.vm_object,
			    &ref_tab,
			    &ref_len);

      entry = entry->next;
      continue;
    }

    /* Here on entry being newly wired */

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"                   entry being newly wired\n");
#endif

    if ((entry->eflags & (MAP_ENTRY_IS_A_MAP|MAP_ENTRY_IS_SUB_MAP)) == 0) {
      int copyflag;

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   inside 'if' block\n");
#endif

      copyflag = entry->eflags & MAP_ENTRY_NEEDS_COPY;
      if (copyflag && ((entry->protection & VM_PROT_WRITE) != 0)) {

#ifdef DEBUG_HSL
	log(LOG_DEBUG,
	    "                   make shadow\n");
#endif

	vm_object_shadow(&entry->object.vm_object,
			 &entry->offset,
			 OFF_TO_IDX(entry->end
				    - entry->start));
	entry->eflags &= ~MAP_ENTRY_NEEDS_COPY;

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   shadow len : 0x%x\n",
	  OFF_TO_IDX(entry->end - entry->start));
#endif

      } else if (entry->object.vm_object == NULL) {
#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   create default object\n");
#endif

	entry->object.vm_object =
	  vm_object_allocate(OBJT_DEFAULT,
			     OFF_TO_IDX(entry->end - entry->start));
	entry->offset = (vm_offset_t) 0;

      }
      default_pager_convert_to_swapq(entry->object.vm_object);
    }

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   clipping entry\n");
#endif

    vm_map_clip_start(map, entry, start);
    vm_map_clip_end(map, entry, end);

    entry->wired_count++;
    entry->eflags |= MAP_ENTRY_USER_WIRED;

    /* First we need to allow map modifications */
    lock_set_recursive(&map->lock);
    lock_write_to_read(&map->lock);

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   vm_fault_user_wire(start=0x%x size=0x%x)\n",
	  (int) entry->start,
	  (int) (entry->end - entry->start));
#endif

    rv = vm_fault_user_wire(map, entry->start, entry->end);
    if (rv) {
      entry->wired_count--;
      entry->eflags &= ~MAP_ENTRY_USER_WIRED;

      lock_clear_recursive(&map->lock);
      vm_map_unlock(map);
				
      (void) vm_map_user_pageable(map, start, entry->start, TRUE);
      return rv;
    }

    lock_clear_recursive(&map->lock);
    vm_map_unlock(map);
    vm_map_lock(map);

    goto rescan;
  }

  vm_map_unlock(map);
  return 0;
}

#else

int
slrpv_prot_wire(prot_addr, prot_len, ref_tab, ref_len, proc)
                vm_offset_t           prot_addr;
                vm_size_t             prot_len;
		prot_range_t         *ref_tab;
		int                  *ref_len;
                struct proc          *proc;
{
  /* SEE vm_map_user_pageable() */

  struct proc *p;

  vm_offset_t addr;
  vm_size_t size, pageoff;
  /* int error; */

  register vm_map_t map;
  register vm_offset_t start;
  register vm_offset_t end;

  vm_map_entry_t entry;
  vm_map_entry_t start_entry;
  vm_offset_t estart;
  int rv;

  /* ref_tab only acts like a "C" language rvalue. Convert it to a lvalue. */
  prot_range_t *ref_tab_p = ref_tab;

  addr = prot_addr;
  size = prot_len;
  p    = proc;

  pageoff = (addr & PAGE_MASK);
  addr -= pageoff;
  size += pageoff;
  size = (vm_size_t) round_page(size);

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "slrpv_prot_wire(): addr=0x%x size=0x%x\n",
      addr,
      size);
#endif

  /* disable wrap around */
  if (addr + size < addr)
    return EINVAL;

  /*
  if (atop(size) + cnt.v_wire_count > vm_page_max_wired)
    return EAGAIN;

#ifdef pmap_wired_count
  if (size + ptoa(pmap_wired_count(vm_map_pmap(&p->p_vmspace->vm_map))) >
      p->p_rlimit[RLIMIT_MEMLOCK].rlim_cur)
    return EAGAIN;
#else
  error = suser(p->p_ucred, &p->p_acflag);
  if (error)
    return error;
#endif
  */

  map   = &proc->p_vmspace->vm_map;
  start = addr;
  end   = addr + size;

/*
 * Implement the semantics of mlock
 */

  vm_map_lock(map);
  VM_MAP_RANGE_CHECK(map, start, end);

  if (vm_map_lookup_entry(map, start, &start_entry) == FALSE) {
    vm_map_unlock(map);
    return ENOMEM /* KERN_INVALID_ADDRESS */;
  }

#if 0
  /*
   * Because of the possiblity of blocking, etc.  We restart
   * through the process's map entries from beginning so that
   * we don't end up depending on a map entry that could have
   * changed.
   */
 rescan:
#endif

  entry = start_entry;

  while ((entry != &map->header) && (entry->start < end)) {

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"slrpv_prot_wire(): current entry = 0x%x (start=0x%x size=0x%x)\n",
	(int) entry,
	(int) entry->start,
	entry->end - entry->start);
#endif

    if (entry->eflags & MAP_ENTRY_USER_WIRED) {
      /* Entry already wired */

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   entry already wired - obj_size = 0x%x\n",
	  (int) IDX_TO_OFF(entry->object.vm_object->size));
#endif

      /* We must wire the object if this has not already been done */
      slrpv_prot_ref_object(entry->object.vm_object,
			    &ref_tab_p,
			    ref_len);

      entry = entry->next;
      continue;
    }
			
    if (entry->wired_count != 0) {
#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   wired_count != 0\n");
#endif

      entry->wired_count++;
      entry->eflags |= MAP_ENTRY_USER_WIRED;

      /* We must wire the object if this has not already been done */
      slrpv_prot_ref_object(entry->object.vm_object,
			    &ref_tab,
			    &ref_len);

      entry = entry->next;
      continue;
    }

    /* Here on entry being newly wired */

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"                   entry being newly wired\n");
#endif

    if ((entry->eflags & (MAP_ENTRY_IS_A_MAP|MAP_ENTRY_IS_SUB_MAP)) == 0) {
      int copyflag;

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   inside 'if' block\n");
#endif

      copyflag = entry->eflags & MAP_ENTRY_NEEDS_COPY;
      if (copyflag && ((entry->protection & VM_PROT_WRITE) != 0)) {

#ifdef DEBUG_HSL
	log(LOG_DEBUG,
	    "                   make shadow\n");
#endif

#if 0
	vm_object_shadow(&entry->object.vm_object,
			 &entry->offset,
			 OFF_TO_IDX(entry->end
				    - entry->start));
#endif
	vm_object_shadow(&entry->object.vm_object,
			 &entry->offset,
			 atop(entry->end - entry->start));

	entry->eflags &= ~MAP_ENTRY_NEEDS_COPY;

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   shadow len : 0x%x\n",
	  OFF_TO_IDX(entry->end - entry->start));
#endif

      } else if (entry->object.vm_object == NULL) {
#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   create default object\n");
#endif

	entry->object.vm_object =
#if 0
	  vm_object_allocate(OBJT_DEFAULT,
			     OFF_TO_IDX(entry->end - entry->start));
#endif
	  vm_object_allocate(OBJT_DEFAULT,
			     atop(entry->end - entry->start));

	entry->offset = (vm_offset_t) 0;

      }
      default_pager_convert_to_swapq(entry->object.vm_object);
    }

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   clipping entry\n");
#endif

    vm_map_clip_start(map, entry, start);
    vm_map_clip_end(map, entry, end);

    entry->wired_count++;
    entry->eflags |= MAP_ENTRY_USER_WIRED;
    estart = entry->start;

    /* First we need to allow map modifications */
#if 0
    lock_set_recursive(&map->lock);
    lock_write_to_read(&map->lock);
#endif
    vm_map_set_recursive(map);
    vm_map_lock_downgrade(map);
    map->timestamp++;

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "                   vm_fault_user_wire(start=0x%x size=0x%x)\n",
	  (int) entry->start,
	  (int) (entry->end - entry->start));
#endif

    rv = vm_fault_user_wire(map, entry->start, entry->end);
    if (rv) {
      entry->wired_count--;
      entry->eflags &= ~MAP_ENTRY_USER_WIRED;

#if 0
      lock_clear_recursive(&map->lock);
#endif
      vm_map_clear_recursive(map);
      vm_map_unlock(map);
				
      (void) vm_map_user_pageable(map, start, entry->start, TRUE);
      return rv;
    }

#if 0
    lock_clear_recursive(&map->lock);
    vm_map_unlock(map);
    vm_map_lock(map);
#endif

    vm_map_clear_recursive(map);
    if (vm_map_lock_upgrade(map)) {
      vm_map_lock(map);
      if (vm_map_lookup_entry(map, estart, &entry)
	  == FALSE) {
	vm_map_unlock(map);
	(void) vm_map_user_pageable(map,
				    start,
				    estart,
				    TRUE);
	log(LOG_ERR,
	    "MAJOR ERROR INSIDE slrpv_prot_wire()\n");
	return KERN_INVALID_ADDRESS;
      }
    }
    vm_map_simplify_entry(map, entry);
  }

  map->timestamp++;
  vm_map_unlock(map);
  return 0;
}

#endif

/*
 ************************************************************
 * slrpv_prot_garbage_collection() : collect garbage
 * inside the protection map.
 ************************************************************
 */

void
slrpv_prot_garbage_collection(void)
{
  int s, t;
  int i, j;
  int res;
  vm_map_entry_t vms_entry;

  if (!vms) {
    log(LOG_ERR,
	"SLRP/V not initialized\n");
    return;
  }

  vm_map_lock(&vms->vm_map);

  /*
   * We must increase the processor level to have mutual exclusion for
   * the accesses to the post treatment locks.
   */
  s = splhigh();

  /*
   * Check if the prot_map_ref_tab tables in pending send and pending
   * receive entries are stable.
   */

  for (i = 0; i < PENDING_SEND_SIZE; i++)
    if (pending_send[i].valid != INVALID &&
	pending_send[i].post_treatment_lock == 1) {
      splx(s);
      vm_map_unlock(&vms->vm_map);
      log(LOG_INFO,
	  "Garbage collection not processed : post treatment not completed\n");
      return;
    }

  for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
    if ((*pending_receive)[i].valid != INVALID &&
	(*pending_receive)[i].post_treatment_lock == 1) {
      splx(s);
      vm_map_unlock(&vms->vm_map);
      log(LOG_INFO,
	  "Garbage collection not processed : post treatment not completed\n");
      return;
    }

  /* Try to destroy each entry in the protection map */

  vms_entry = vms->vm_map.header.next;

  while (vms_entry != &vms->vm_map.header) {
    vm_offset_t vms_start, vms_end;

    vms_start = vms_entry->start;
    vms_end   = vms_entry->end;

    for (i = 0; i < PENDING_SEND_SIZE; i++)
      if (pending_send[i].valid != INVALID)
	for (j = 0; j < PROT_MAP_REF_TAB_SIZE; j++)
	  if (INSIDE_RANGE(pending_send[i].prot_map_ref_tab[j].start,
			   pending_send[i].prot_map_ref_tab[j].start +
			   pending_send[i].prot_map_ref_tab[j].size,
			   vms_start,
			   vms_end)) {
	    /* The vm map entry is referenced, do not destroy it */
	    vms_entry = vms_entry->next;
	    goto restart;
	  }

    for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
      if ((*pending_receive)[i].valid != INVALID)
	for (j = 0; j < PROT_MAP_REF_TAB_SIZE; j++)
	  if (INSIDE_RANGE((*pending_receive)[i].prot_map_ref_tab[j].start,
			   (*pending_receive)[i].prot_map_ref_tab[j].start +
			   (*pending_receive)[i].prot_map_ref_tab[j].size,
			   vms_start,
			   vms_end)) {
	    /* The vm map entry is referenced, do not destroy it */
	    vms_entry = vms_entry->next;
	    goto restart;
	  }

    /* The vm map entry is not referenced, we can destroy it */

    /*
     * We don't use vm_map_remove() because we should then release the lock :
     * vm_map_remove() locks the map... Releasing the lock means that the
     * map entries list may change, and that's not what we want...
     * Then we use vm_map_delete() that is designed for internal use by MACH,
     * but it is not labeled static (in the FreeBSD version at least)...
     */

    splx(s);
    t = splvm();

    VM_MAP_RANGE_CHECK(&vms->vm_map, vms_start, vms_end);
    log(LOG_INFO,
	"Garbage collection: deleting range [0x%x, 0x%x]\n",
	vms_start,
	vms_end);
    res = vm_map_delete(&vms->vm_map, vms_start, vms_end);

    splx(t);
    s = splhigh();

    /* Since we modified the map entries list, we must start
       at the beginning of the list */

    vms_entry = vms->vm_map.header.next;

  restart:
  }

  vm_map_unlock(&vms->vm_map);

  log(LOG_INFO,
      "Garbage collection correctly processed\n");
}


/*
 ************************************************************
 * slrpv_set_last_callback(dest, channel, fct, param, proc) :
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
slrpv_set_last_callback(dest, channel, fct, param, proc)
                        node_t dest;
			channel_t channel;
			boolean_t (*fct) __P((caddr_t));
			caddr_t param;
			struct proc *proc;
{
  return slrpp_set_last_callback(dest, channel, fct, param, proc);
}


/*
 ************************************************************
 * slrpv_open_channel(dest, channel, classname, proc) : open a channel.
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
slrpv_open_channel(dest, channel, classname, proc)
                   node_t dest;
		   channel_t channel;
		   appclassname_t classname;
		   struct proc *proc;
{
  return slrpp_open_channel(dest, channel, classname, proc);
}


/*
 ************************************************************
 * slrpv_shutdown0_channel(dest, channel, seq_send, seq_recv, proc) :
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
slrpv_shutdown0_channel(dest, channel, seq_send, seq_recv, proc)
                        node_t dest;
			channel_t channel;
			seq_t *seq_send, *seq_recv;
			struct proc *proc;
{
  return slrpp_shutdown0_channel(dest, channel, seq_send, seq_recv, proc);
}


/*
 ************************************************************
 * slrpv_shutdown1_channel(dest, channel, seq_send, seq_recv, proc) :
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
slrpv_shutdown1_channel(dest, channel, seq_send, seq_recv, proc)
                        node_t dest;
			channel_t channel;
			seq_t seq_send, seq_recv;
			struct proc *proc;
{
  return slrpp_shutdown1_channel(dest, channel, seq_send, seq_recv, proc);
}


/*
 ************************************************************
 * slrppv_close_channel(dest, channel, proc) :
 * shutdown a channel.
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
slrpv_close_channel(dest, channel, proc)
                    node_t dest;
		    channel_t channel;
		    struct proc *proc;
{
  return slrpp_close_channel(dest, channel, proc);
}

#endif

