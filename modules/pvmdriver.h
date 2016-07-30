/* $Id: pvmdrv.h,v 1.00 1998/03/26 00:00:00  kmana $ */

#ifndef _PVMDRIVER_H_
#define _PVMDRIVER_H_

#define PVMMAJOR 123

#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif
#include "put.h"
#include "ddslrpv.h"
#include "cmem.h"
#include "../MPC_OS/mpcshare.h"

/* Size of CMEM slot for PVM status */
#define STATUS_SIZE_SPACE 1024*1024

/********************************************************/
/*	          PVM-SLRV   I/O ciontrols              */
/********************************************************/

typedef struct _pvm_contigmem {
  size_t	size;			/* required / obtained size */
  volatile caddr_t       vptr;                   /* processus virtual address */
  volatile caddr_t	kptr;			/* kernel virtual address */
  volatile caddr_t	pptr;			/* physical address */
  char		name[CMEM_NAMELEN];	/* name for the slot */
  int		slot;			/* slot number */
} pvm_contigmem_t;

typedef struct _sendrecv {
  node_t dest;
  channel_t channel;
  caddr_t data;
  volatile caddr_t status;
  size_t  size;
  char sr;                /* send or receive bit */
  char inter;             /* callback interruption bit */
  int num;
} sendrecv;


  
void Callbacksnd(int);
void Callbackrcv(int);

#define PVMHSLGETNODE    _IOR('P', 17, int)
#define PVM_GETSTATUSMEM	       	_IOWR('P', 22, struct _pvm_contigmem)
#define PVM_FREEMEM		_IOW ('P', 23, int)
#define PVM_GET_STATUS	       	_IOWR('P', 24, int)

#define HSLSLRVSENDRECV    _IOWR('P', 18, struct _sendrecv)

typedef struct _resetpp{  
  node_t node;
  channel_t channel;
}  resetpp;

#define HSLRESET    _IOWR('P',19,struct _resetpp)

typedef struct _hsllock{
  vm_offset_t  addr;
  vm_size_t  size;
}  hsllock;

#define HSLLOCK      _IOWR('P',20,struct _hsllock)
#define HSLUNLOCK    _IOWR('P',21,struct _hsllock)

/*********************************************************/
/*           PVM-SLRV  I/O ctls  end                    */
/*********************************************************/

#endif

