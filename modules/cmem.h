
/* $Id: cmem.h,v 1.2 1998/12/20 20:23:09 alex Exp $ */

#ifndef _CMEMDRIVER_H_
#define _CMEMDRIVER_H_

#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif

#define CMEM_NSLOTS  10
#define CMEM_NAMELEN 9
#define CMEM_SIZE (10 * 1024 * 1024)

typedef struct _cmem_slot {
  struct _cmem_slot *next_slot;
  struct _cmem_slot *prev_slot;
  caddr_t     phys_start;
  vm_offset_t virt_start;
  vm_size_t   len;
  char name[CMEM_NAMELEN];
} cmem_slot_t;

extern caddr_t cmem_physaddr __P((int));

extern vm_offset_t cmem_virtaddr __P((int));

extern int cmem_getmem __P((vm_size_t, char *));

extern int cmem_releasemem __P((int));

#endif
