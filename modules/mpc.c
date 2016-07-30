
/* Alexandre Fenyo (alex@asim.lip6.fr) */

#include "mpc.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/malloc.h>

#include <pci/pcireg.h>
#include <pci/pcivar.h>

#include <sys/syslog.h>

void (*mpc_dynamic_handler)(void *);

static const char *mpc_probe(pcici_t tag, pcidi_t type);
static void mpc_attach (pcici_t tag, int unit);
static	u_long	mpc_count;

static d_open_t	mpc_open;
static d_close_t mpc_close;
static d_ioctl_t mpc_ioctl;

#define CDEV_MAJOR 115
static struct cdevsw mpcdevsw = {
  mpc_open,
  mpc_close,
  noread,
  nowrite,
  mpc_ioctl,
  nullstop,
  noreset,
  nodevtotty,
  seltrue,
  nommap,
  nostrategy,
  "mpc",
  NULL,
  -1
};

static MALLOC_DEFINE(M_MPC, "mpc", "MPC related");

static int
mpc_open(dev_t dev, int flag, int mode, struct proc *p)
{
  return 0;
}

static int
mpc_close(dev_t dev, int flag, int mode, struct proc *p)
{ 
  return 0;
}

static int
mpc_ioctl(dev_t dev, u_long cmd, caddr_t arg, int flag, struct proc *pr)
{
  return 0;
}

/*
 * PCI initialization stuff
 */

static struct pci_device mpc_device = {
  "mpc",
  mpc_probe,
  mpc_attach,
  &mpc_count,
  NULL
};

DATA_SET(pcidevice_set, mpc_device);

static const char *
mpc_probe(pcici_t tag, pcidi_t typea)
{
  u_int id;

  id = pci_conf_read(tag, PCI_ID_REG);

  if (!id)
    return "ASIM-LIP6 FastHSL driver : alex@asim.lip6.fr";

  return 0;
}

static void
mpc_interrupt_handler(void *arg)
{
  if (mpc_dynamic_handler)
    mpc_dynamic_handler(arg);
}

static void
mpc_attach(pcici_t tag, int unit)
{
  dev_t cdev = makedev(CDEV_MAJOR, unit);

  if (!unit)
    cdevsw_add(&cdev, &mpcdevsw, NULL);

  mpc_dynamic_handler = NULL;
  pci_map_int(tag, mpc_interrupt_handler, NULL, &net_imask);
}

