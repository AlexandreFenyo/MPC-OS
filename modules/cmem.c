
/* $Id: cmem.c,v 1.5 2000/02/25 17:57:36 alex Exp $ */

/*******************************************************************
 * AUTHOR : Alexandre Fenyö (Alexandre.Fenyo@lip6.fr)              *
 *******************************************************************/

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#include <sys/cdefs.h>
#include <sys/types.h>

#include <sys/sysent.h>
#include <sys/param.h>
#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif
#include <sys/systm.h>

#ifndef WITH_KLD
#define ACTUALLY_LKM_NOT_KERNEL
#endif
#include <sys/signalvar.h>
#include <sys/conf.h>

#include <sys/mount.h>
#include <sys/exec.h>

#ifndef WITH_KLD
#include <sys/lkm.h>
#include <a.out.h>
#endif

#include <sys/file.h>
#include <sys/syslog.h>
#include <vm/vm.h>

#include <vm/vm_extern.h>

#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <vm/lock.h>
#endif

#include <vm/vm_prot.h>
#include <vm/pmap.h>
#include <vm/vm_page.h>

#include <vm/vm_map.h>
#include <machine/pmap.h>
#include <sys/errno.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>

#ifdef _SMP_SUPPORT
#include <sys/uio.h>
#endif

#ifdef WITH_KLD
#include <sys/kernel.h>
#include <sys/module.h>
#endif

#include "../MPC_OS/mpcshare.h"

#include "data.h"
#include "cmem.h"

static int dev_open();
static int dev_close();
static int dev_read();
static int dev_write();
static int dev_ioctl();

#ifdef _SMP_SUPPORT
static int
noselect(dev, rw, p)
        dev_t dev;
        int rw;
        struct proc *p;
{
        printf("noselect(0x%x) called\n", dev);
        /* XXX is this distinguished from 1 for data available? */
        return (ENXIO);
}
#endif

static struct cdevsw cdev = {
  dev_open,
  dev_close,
  dev_read,
  dev_write,
  dev_ioctl,
#ifndef _SMP_SUPPORT
  nxstop,
#else
  nostop,
#endif
#ifndef _SMP_SUPPORT
  nxreset,
#else
  noreset,
#endif
#ifndef _SMP_SUPPORT
  nxdevtotty,
#else
  nodevtotty,
#endif
#ifndef _SMP_SUPPORT
  nxselect,
#else
  noselect,
#endif
#ifndef _SMP_SUPPORT
  nxmmap,
#else
  nommap,
#endif
#ifndef _SMP_SUPPORT
  nxstrategy,
#else
  nostrategy,
#endif
  "cmem",
  (struct bdevsw *) NULL,
  -1
};

#ifndef _SMP_SUPPORT

/*
 * We redefine MOD_DEV because of a lack of braces in its definition in
 * /sys/sys/lkm.h that generates a warning with gcc -Wall.
 * Finally, we don't redefine MOD_DEV because it has been modified many
 * times...
 */

#ifndef WITH_KLD

#if 0

#ifdef MOD_DEV
#undef MOD_DEV
#endif /* MOD_DEV */

#define MOD_DEV(name,devtype,devslot,devp)        \
        MOD_DECL(name);                           \
        static struct lkm_dev name ## _module = { \
                LM_DEV,                           \
                LKM_VERSION,                      \
                #name ## "_mod",                  \
                devslot,                          \
                devtype,                          \
                { (void *)devp }                  \
        }

#endif /* 0 */

#endif /* WITH_KLD */

#endif /* _SMP_SUPPORT */

#ifndef WITH_KLD
MOD_DEV(cmemdriver,
	LM_DT_CHAR,
	-1,
	&cdev);
#endif /* WITH_KLD */

static caddr_t     phys_start;
static vm_offset_t virt_start;
static vm_size_t   cmem_size;

static cmem_slot_t slots[CMEM_NSLOTS];
static cmem_slot_t *first_slot;

static int txt_offset;
static char txt_info[10000];

#define PRINT_DEV(X) \
  sprintf(txt_info + strlen(txt_info), X); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define PRINT_DEV_1(X, Y) \
  sprintf(txt_info + strlen(txt_info), X, Y); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define PRINT_DEV_5(X, Y, Z, T, U, V) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z, T, U, V); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define FORMAT_PTR   1
#define FORMAT_SHORT 2
#define FORMAT_INT   3


/*
 ************************************************************
 * format(type, value) : format a long.
 * type  : type of format.
 * value : value to format.
 * 
 * return value : resulting string.
 ************************************************************
 */

static char *
format(type, value)
       int  type;
       long value;
{
  char tmp[256];
  int i;
  static char strings[10][256];
  static int current = 0;

  if (current == 10)
    current = 0;

  for (i = 0;
       i < sizeof strings[current];
       i++)
    strings[current][i] = ' ';

  switch (type) {
  case FORMAT_PTR:
    sprintf(tmp,
	    "%p",
	    (caddr_t) value);
    strcpy(strings[current] + 10 - strlen(tmp),
	 tmp);
    break;

  case FORMAT_SHORT:
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 4 - strlen(tmp),
	 tmp);
    break;

  case FORMAT_INT:
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 8 - strlen(tmp),
	 tmp);
    break;

  default:
    sprintf(strings[current],
	    "ERROR");
  }

  return strings[current++];
}


/*
 ************************************************************
 * format_str(type, value) : format a string.
 * type  : type of format.
 * value : string to format.
 * 
 * return value : resulting string.
 ************************************************************
 */

static char *
format_str(size, value)
          int   size;
          char *value;
{
  char tmp[256];
  int i;
  static char strings[10][256];
  static int current = 0;

  if (current == 10)
    current = 0;

  for (i = 0;
       i < sizeof strings[current];
       i++)
    strings[current][i] = ' ';

  sprintf(tmp,
	  "%s",
	  value);
  strcpy(strings[current] + size - strlen(tmp),
	 tmp);

  return strings[current++];
}


/*
 ************************************************************
 * cmem_update() : log internal state onto the console.
 ************************************************************
 */

static void
cmem_update()
{
  int cpt;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "cmem_update()\n");
#endif

  txt_info[0] = 0;

  PRINT_DEV("/dev/cmem : low level driver for contig. space\n\n");

  PRINT_DEV_1("physical start : %s\n",
	      format(FORMAT_PTR,
		     phys_start));
  PRINT_DEV_1("virtual  start : %s\n",
	      format(FORMAT_PTR,
		     virt_start));
  PRINT_DEV_1("length         :   %s\n\n",
	      format(FORMAT_INT,
		     cmem_size));

  PRINT_DEV("+------+------------+------------+----------+----------+\n");
  PRINT_DEV("| slot | phys start | virt start |     size |     name |\n");
  PRINT_DEV("+------+------------+------------+----------+----------+\n");

  for (cpt = 0; cpt < CMEM_NSLOTS; cpt++) {
    PRINT_DEV_5("| %s | %s | %s | %s | %s |\n",
		format(FORMAT_SHORT,
		       cpt),
		format(FORMAT_PTR,
		       slots[cpt].phys_start),
		format(FORMAT_PTR,
		       slots[cpt].virt_start),
		format(FORMAT_INT,
		       slots[cpt].len),
		format_str(8,
			   slots[cpt].name));
    PRINT_DEV("+------+------------+------------+----------+----------+\n");
  }
}

/*
 ************************************************************
 * cmem_releasemem(id) : release a slot of contig. memory.
 * id : the slot number.
 *
 * return value : -1 on error.
 *                 0 on success.
 ************************************************************
 */

int
cmem_releasemem(id)
                int id;
{
  int s;

  s = splhigh();

  if (id < 0 ||
      id >= CMEM_NSLOTS ||
      !slots[id].len) {
    splx(s);
    return -1;
  }

  if (first_slot == slots + id)
    first_slot = slots[id].next_slot;

  if (slots[id].next_slot)
    slots[id].next_slot->prev_slot = slots[id].prev_slot;
  if (slots[id].prev_slot)
    slots[id].prev_slot->next_slot = slots[id].next_slot;

  slots[id].next_slot = NULL;
  slots[id].prev_slot = NULL;
  slots[id].phys_start = NULL;
  slots[id].virt_start = NULL;
  slots[id].name[0] = 0;
  slots[id].len = 0;

  cmem_update();

  splx(s);

  return 0;
}


/*
 ************************************************************
 * cmem_virtaddr(slot) : get the virtual start address of a slot.
 * slot : the slot number.
 * 
 * return value : virtual start address of the slot.
 ************************************************************
 */

vm_offset_t
cmem_virtaddr(int slot)
{
  return slots[slot].virt_start;
}


/*
 ************************************************************
 * cmem_phys_addr(slot) : get the physical start address of a slot.
 * slot : the slot number.
 * 
 * return value : physical start address of the slot.
 ************************************************************
 */

caddr_t
cmem_physaddr(int slot)
{
  return slots[slot].phys_start;
}


/*
 ************************************************************
 * cmem_getmem(size, name) : get a slot of contig. memory.
 * size : the size of the slot.
 * name : a string to identify the user of the slot.
 *
 * return values : -1 on error.
 *                 the slot number on success.
 ************************************************************
 */

int
cmem_getmem(size, name)
            vm_size_t size;
            char *name;
{
  int s;
  int nslot;
  cmem_slot_t *current;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "cmem_getmem(%p, %s)\n", (int) size, name);
#endif

  s = splhigh();

  size = round_page(size);

  for (nslot = 0;
       nslot < CMEM_NSLOTS;
       nslot++)
    if (!slots[nslot].phys_start)
      break;

  if (nslot == CMEM_NSLOTS) {
    splx(s);
    return -1;
  }

  if (!name) {
    splx(s);
    return -1;
  }

  strncpy(slots[nslot].name, name, sizeof slots[nslot].name);
  slots[nslot].name[sizeof(slots[nslot].name) - 1] = 0;

  if (!first_slot) {
    if (size > cmem_size) {
      splx(s);
      return -1;
    }

    first_slot = slots;
    first_slot->next_slot = NULL;
    first_slot->prev_slot = NULL;
    first_slot->phys_start = phys_start;
    first_slot->virt_start = virt_start;
    first_slot->len = size;

    cmem_update();

    bzero((void *) virt_start, size);
    splx(s);
    return 0;
  }

  for (current = first_slot;
       current->next_slot;
       current = current->next_slot)
    if (current->next_slot->virt_start - current->virt_start
	- current->len >= size) break;

  if (current->next_slot) {
    slots[nslot].next_slot = current->next_slot;
    slots[nslot].prev_slot = current;
    slots[nslot].phys_start = current->phys_start + current->len;
    slots[nslot].virt_start = current->virt_start + current->len;
    slots[nslot].len = size;

    current->next_slot->prev_slot = slots + nslot;
    current->next_slot = slots + nslot;
  } else {
    if ((virt_start + cmem_size) -
	(current->virt_start + current->len) < size) {
      splx(s);
      return -1;
    }

    slots[nslot].next_slot = NULL;
    slots[nslot].prev_slot = current;
    slots[nslot].phys_start = current->phys_start + current->len;
    slots[nslot].virt_start = current->virt_start + current->len;
    slots[nslot].len = size;

    current->next_slot = slots + nslot;
  }

  cmem_update();

  bzero((void *) slots[nslot].virt_start, slots[nslot].len);
  splx(s);
  return nslot;
}


/*
 ************************************************************
 * dev_open() : handler for the open operation on the driver.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_open()
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_open\n");
#endif

  txt_offset = 0;

  return 0;
}


/*
 ************************************************************
 * dev_close() : handler for the close operation on the driver.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_close()
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_close\n");
#endif

  txt_offset = 0;

  return 0;
}


/*
 ************************************************************
 * dev_read() : handler for the read operation on the driver.
 * dev  : device.
 * uio  : read vector.
 * flag : flags.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_read(dev, uio, flag)
         dev_t dev;
         struct uio *uio;
         int flag;
{
  int res;
  int len;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_read\n");
#endif

  len = uio->uio_iov->iov_len;
  len = MIN(len, strlen(txt_info) - txt_offset);
  if (!len) return 0;

  res = uiomove(txt_info + txt_offset, len, uio);
  txt_offset += len;

  return res;
}


/*
 ************************************************************
 * dev_write() : handler for the write operation on the driver.
 * dev  : device.
 * uio  : write vector.
 * flag : flags.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_write(dev, uio, flag)
         dev_t dev;
         struct uio *uio;
         int flag;
{
  return 0;
}


/*
 ************************************************************
 * dev_ioctl() : handler for the ioctl operation on the driver.
 * dev  : device.
 * com  : type of IOCTL.
 * data : R/W data passed from/to the process.
 * flag : flags.
 * p    : proc struct of the calling process.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_ioctl(dev, com, data, flag, p)
     dev_t dev;
     int com;
     caddr_t data;
     int flag;
     struct proc *p;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_ioctl\n");
#endif

  switch (com) {}

  return 0;
}

/*
 ************************************************************
 * cmemdriver_load(lkmtp, cmd) : handler for the load operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
#ifndef WITH_KLD
cmemdriver_load(lkmtp, cmd)
#else
lkm_cmemdriver_load(lkmtp, cmd)
#endif
                struct lkm_table *lkmtp;
                int cmd;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Load!\n");
#endif

  first_slot = NULL;
  phys_start = NULL;
  virt_start = NULL;
  cmem_size  = 0;
  bzero(slots,
	sizeof slots);

  cmem_size = round_page(CMEM_SIZE);
  virt_start = vm_page_alloc_contig(cmem_size,
				    0L,
				    -1L,
				    PAGE_SIZE);

  if (!virt_start) {
    log(LOG_WARNING, "Can't allocate cmem space\n");

    cmem_size  = 0;
    phys_start = NULL;
  } else {
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"allocation of cmem space successful : %d bytes\n",
	cmem_size);
#endif

    phys_start = (caddr_t) vtophys(virt_start);
  }

  cmem_update();

  printf("\nCMEM driver loaded with success\n(c) Laboratoire LIP6 / Departement ASIM\n    Universite Pierre et Marie Curie (Paris 6)\n(feel free to contact fenyo@asim.lip6.fr for informations about this driver)\n");

  return 0;
}


/*
 ************************************************************
 * cmemdriver_unload(lkmtp, cmd) : handler for the unload operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
#ifndef WITH_KLD
cmemdriver_unload(lkmtp, cmd)
#else
lkm_cmemdriver_unload(lkmtp, cmd)
#endif
     struct lkm_table *lkmtp;
     int cmd;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Unload!\n");
#endif

  /*
   * We should free the contig. space, but it may crash the system...
   *
   * if (virt_start) {
   * kmem_free(kernel_map, virt_start, cmem_size);
   * }
   */

  return 0;
}


/*
 ************************************************************
 * cmemdriver_stat(lkmtp, cmd) : handler for the stat operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
#ifndef WITH_KLD
cmemdriver_stat(lkmtp, cmd)
#else
lkm_cmemdriver_stat(lkmtp, cmd)
#endif
         struct lkm_table *lkmtp;
         int cmd;
{
  return 0;
}


/*
 ************************************************************
 * cmeminit(lkmtp, cmd) : entry point of the driver.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload).
 * ver   : version of the loading procedure applied to this driver by the system.
 * 
 * return value : status of the operation.
 ************************************************************
 */

#ifndef WITH_KLD

int
cmeminit(lkmtp, cmd, ver)
         struct lkm_table *lkmtp;
         int cmd;
         int ver;
{
#ifndef _SMP_SUPPORT
#define _module cmemdriver_module
  DISPATCH(lkmtp,
	   cmd,
	   ver,
	   cmemdriver_load,
	   cmemdriver_unload,
	   cmemdriver_stat)
#else
  DISPATCH(cmemdriver,
	   lkmtp,
	   cmd,
	   ver,
	   cmemdriver_load,
	   cmemdriver_unload,
	   cmemdriver_stat)
#endif
}

#else

static int
cmemdriver_load(mod, cmd, arg)
                module_t mod;
                modeventtype_t cmd;
                void *arg;
{
  int err = 0;

  switch (cmd) {
  case MOD_LOAD:
    printf("Loaded cmem driver (KLD)\n");
    return lkm_cmemdriver_load(NULL, 0);
    break;

  case MOD_UNLOAD:
    printf("Unloaded cmem driver (KLD)\n");
    return lkm_cmemdriver_unload(NULL, 0);
    break;

  default: /* We only understand load/unload */
    err = EINVAL;
    break;
  }

  return err;
}

CDEV_MODULE(cmemdriver_mod, CMEMMAJOR, cdev, cmemdriver_load, 0);

#endif
