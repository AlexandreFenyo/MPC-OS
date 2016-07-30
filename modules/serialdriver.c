
/* $Id: serialdriver.c,v 1.4 2000/02/25 17:57:38 alex Exp $ */

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

#include <vm/vm_kern.h>

#ifdef _SMP_SUPPORT
#include <sys/uio.h>
#endif

#ifdef WITH_KLD
#include <sys/kernel.h>
#include <sys/module.h>
#endif

extern struct pcibus i386pci;
int driver_phys_mem;

#include "../MPC_OS/mpcshare.h"

#include "data.h"

static int dev_open();
static int dev_close();
static int dev_read();
static int dev_ioctl();

#ifdef _SMP_SUPPORT
int
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
#ifndef _SMP_SUPPORT
  nxwrite,
#else
  nowrite,
#endif
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
  "serial",
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
MOD_DEV(serialdriver,
	LM_DT_CHAR,
	-1,
	&cdev);
#endif /* WITH_KLD */

static int
dev_open()
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_open\n");
#endif

  return 0;
}

static int
dev_close()
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_close\n");
#endif

  return 0;
}

static int
dev_read(dev, uio, flag)
         dev_t dev;
         struct uio *uio;
         int flag;
{

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_read\n");
#endif

  return 0;
}

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

  return 0;
}


/*
static int
handler_interrupt(arg)
                  int arg;
{
  return 0;
}
*/

#ifndef WITH_KLD

static int
serialdriver_load(lkmtp, cmd)
     struct lkm_table *lkmtp;
     int cmd;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Load!\n");
#endif

/*
  tag = i386pci.pb_tag(0, 10, 0);
  driver_phys_mem = i386pci.pb_read(tag, PCI_MAP_REG_START);

  log(LOG_DEBUG, "Driver phys. mem : 0x%x\n", driver_phys_mem);


  if (!pci_map_int(tag, &handler_interrupt, NULL, &*//*net_imask*//*tty_imask)) {
    log(LOG_ERR, "Couldn't map interrupt\n");
    return 0;
  }
  log(LOG_DEBUG, "Interrupt correctly mapped\n");
*/

  return 0;
}

static int
serialdriver_unload(lkmtp, cmd)
     struct lkm_table *lkmtp;
     int cmd;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Unload!\n");
#endif

/*
  if (!pci_unmap_int(tag)) {
    log(LOG_ERR, "Couldn't unmap interrupt\n");
    return 0;
  }

  log(LOG_DEBUG, "Interrupt correctly unmapped");
*/

  return 0;
}

static int
serialdriver_stat(lkmtp, cmd)
                  struct lkm_table *lkmtp;
                  int cmd;
{
  return 0;
}

int
serialinit(lkmtp, cmd, ver)
           struct lkm_table *lkmtp;
           int cmd;
           int ver;
{
#ifndef _SMP_SUPPORT
#define _module serialdriver_module
  DISPATCH(lkmtp,
	   cmd,
	   ver,
	   serialdriver_load,
	   serialdriver_unload,
	   serialdriver_stat)
#else
  DISPATCH(serialdriver,
	   lkmtp,
	   cmd,
	   ver,
	   serialdriver_load,
	   serialdriver_unload,
	   serialdriver_stat)
#endif
}

#else

static int
serialdriver_load(mod, cmd, arg)
                  module_t mod;
                  modeventtype_t cmd;
                  void *arg;
{
  int err = 0;

  switch (cmd) {
  case MOD_LOAD:
    printf("Loaded serial driver (KLD)\n");
    break;

  case MOD_UNLOAD:
    printf("Unloaded serial driver (KLD)\n");
    break;

  default: /* We only understand load/unload */
    err = EINVAL;
    break;
  }

  return err;
}

CDEV_MODULE(serialdriver_mod, SERIALMAJOR, cdev, serialdriver_load, 0);

#endif

