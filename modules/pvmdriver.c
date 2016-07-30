/* $Id: pvmdriver.c,v 1.00 1998/03/26 00:00:00 kmana Exp $ */

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
/*#include <vm/vm_page.h>*/
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

/* required by psignal() */
#include <sys/types.h>
#include <sys/signalvar.h>


/*
#ifdef DEBUG_HSL
#undef DEBUG_HSL
#endif
*/

extern struct pcibus i386pci;
int driver_phys_mem;

#include "../MPC_OS/mpcshare.h"

#include "data.h"
#include "pvmdriver.h"  /*  C'est notre driver.h, nos structs...*/


/* CMEM Slot for PVM status */
static pvm_contigmem_t cm_status;

/* Count busy status */
static int cpt_status = -1;
static int cp_int_send = 0;
static int cp_int_recv = 0;

static char txt_info[10000];

#define PRINT_DEV(X) \
  sprintf(txt_info + strlen(txt_info), X); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

#define PRINT_DEV_1(X, Y) \
  sprintf(txt_info + strlen(txt_info), X, Y); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

#define PRINT_DEV_2(X, Y, Z) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

#define PRINT_DEV_3(X, Y, Z, T) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z, T); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

#define PRINT_DEV_4(X, Y, Z, T, U) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z, T, U); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

#define PRINT_DEV_5(X, Y, Z, T, U, V) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z, T, U, V); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info, "--TRUNCATED--\n"); \
  }

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

#if 0
static struct cdevsw cdev = {
  dev_open,
  dev_close,
  dev_read,
  nxwrite,
  dev_ioctl,
  nxstop,
  nxreset,
  nxdevtotty,
  nxselect,
  nxmmap,
  nxstrategy,
  "serial",
  (struct bdevsw *) NULL,
  -1
};
#endif

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
#endif

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
MOD_DEV(pvmdriver,
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
		/*kmana*/

  return 0;
}

static int
dev_close()
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_close\n");
#endif
                  /*kmana*/
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
		/*kmana*/
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

/*#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_ioctl\n");
#endif
	*/		/*kmana*/  
 sendrecv *sndr;  
 pvm_contigmem_t *cm;
 int *pi;
 hsllock *lck;
 int res,ss;
 int r;

/*-----------------------------------------------*/

  switch (com) {
 
 case PVMHSLGETNODE:
    TRY("HSLGETNODE")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLGETNODE\n");
#endif

    *(int *) data = put_get_node(minor(dev));
    return 0;
    break;

  case PVM_GET_STATUS:
    TRY("PVM_GET_STATUS");
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL PVM_GET_STATUS\n");
#endif

    cpt_status++;
    if (cpt_status > STATUS_SIZE_SPACE - 1)
      {
	cpt_status = 0;
	log(LOG_DEBUG, "Attention : Il n'y avait plus de status ; on est reparti a zero\n");
      }

    *(int *) data = cpt_status;

    return 0;
    break;

    /* The slot is get while the driver is loaded */
  case PVM_GETSTATUSMEM:
    TRY("PVM_GETSTATUSMEM");
    cm = (pvm_contigmem_t *) data;
    
    if (cm->size > cm_status.size || !cm_status.kptr || !cm_status.pptr || cm_status.slot == -1) 
      {
	PRINT_DEV_3(" - PVM_GETSTATUSMEM error (slot %d) (size %d) (name %s)\n",
		    cm_status.slot, cm_status.size, cm_status.name);
	return -1;
      }
    
    strcpy(cm->name, cm_status.name);
    cm->size = (size_t) cm_status.size;
    cm->kptr = (caddr_t) cm_status.kptr;
    cm->pptr = (caddr_t) cm_status.pptr;
    cm->slot = (int) cm_status.slot;

    return 0;
    break;

   case PVM_FREEMEM:
    TRY("PVM_FREEMEM");
    pi = (int *) data;		/* data is a slot number */
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL PVM_FREEMEM (slot %d)\n", *pi);
#endif
    cmem_releasemem(*pi);
    return 0;			/* cmem_releasemem always returns 0 */
    break;   

	/*           PVM-SLRV  I/O  ctls      */

case HSLLOCK:
    TRY("HSLLOCK")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLLOCK\n");
#endif

    lck =  (hsllock *) data;
    res = slrpv_mlock(lck->addr,lck->size,p);
    return res;
    /* NOTREACHED */
    break;


 case HSLUNLOCK:
    TRY("HSLUNLOCK")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLUNLOCK\n");
#endif

    lck =  (hsllock *) data;
    res = slrpv_munlock(lck->addr,lck->size,p);
    return res;
    /* NOTREACHED */
    break;


/****************************************************************/
/*                HSL-SLRV-SEND/RECEIVE  IOCTL                  */
/*                   WITH CALLBACK FUNCTION                     */
/****************************************************************/
 case HSLSLRVSENDRECV:
    TRY("HSLSLRVSENDRECV");

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLSLRVSENDRECV\n");
#endif



      sndr = (sendrecv *) data;

/*      log(LOG_DEBUG, "IOCTL sndr-sr=%d sndr-inter=%d sndr-statusnum=%d\n",sndr->sr, sndr->inter, sndr->num); */

    if(sndr->sr == 1)    /* SEND */
      {
        if(sndr->inter == 1)  /* with interrupt */
          {
/*	    log(LOG_DEBUG, "avant slprvsend : sndr-statusnum=%d\n",sndr->num);*/
           res = slrpv_send(sndr->dest,
         	        sndr->channel,
      	                sndr->data,
	                sndr->size,
                        (void (*)(caddr_t, int)) Callbacksnd,(caddr_t) sndr->num , p);
	   if (res) return res;


#ifdef DEBUG_HSL
            log(LOG_DEBUG, "HSLSLRVSENDRECV-SEND-INTERRUPT\n");
#endif
	    ss=splhigh();
	    while(! ( * ((char *) (cm_status.kptr + sndr->num))))
	      {
		r=tsleep(cm_status.kptr + sndr->num, HSLPRI|PCATCH,"DATASENT", 0);
/*		if (r == EINTR) break;*/
	      }

	    * ((char *) (cm_status.kptr + sndr->num)) = (char) 0;
	    splx(ss);
	    	    

            return res;
          }
        else
          {
            res = slrpv_send(sndr->dest,
                             sndr->channel,
                             sndr->data,
                             sndr->size, NULL, NULL, p);
#ifdef DEBUG_HSL
            log(LOG_DEBUG, "HSLSLRVSENDRECV-SEND-NOINTERRUPT\n");
#endif

            return res;

          }
      }
    else  /* RECEIVE */
      {
        if(sndr->inter == 1)  /* with interrupt */
          {
#ifdef DEBUG_HSL
        log(LOG_DEBUG, "HSLSLRVSENDRECV-RECEIVE-INTERRUPT\n");
 log(LOG_DEBUG, "Dest :  %d, channel :  %d,  size : %d\n", sndr->dest, sndr->channel,sndr->size);
#endif

	    /* debug provisoire */
/*	    log(LOG_DEBUG, " SLRPV RECV : Dest :  %d, channel :  %d,  size : %d STATUS = %d\n", sndr->dest, sndr->channel,sndr->size,* ((char *) (cm_status.kptr + sndr->num)));*/

            res = slrpv_recv(sndr->dest,
                             sndr->channel,
                             sndr->data,
                             sndr->size,
                             (void (*)(caddr_t)) Callbackrcv,
                             (caddr_t) sndr->num, p);
#ifdef DEBUG_HSL
        log(LOG_DEBUG, "SLRPV RECV  result :  %d\n",res);
 log(LOG_DEBUG, "Dest :  %d, channel :  %d,  size : %d\n", sndr->dest, sndr->channel,sndr->size);     
#endif
            return res;
          }
        else
          {
#ifdef DEBUG_HSL
            log(LOG_DEBUG, "HSLSLRVSENDRECV-RECEIVE-NOINTERRUPT\n");
 log(LOG_DEBUG, "Dest :  %d, channel :  %d,  size : %d\n", sndr->dest, sndr->channel,sndr->size);
#endif
     
      
       res = slrpv_recv(sndr->dest,
                             sndr->channel,
                             sndr->data,
                             sndr->size,
                             NULL, NULL, p);
#ifdef DEBUG_HSL 
 log(LOG_DEBUG, "slrpv recv result :  %d\n", res);            
  log(LOG_DEBUG, "Dest :  %d, channel :  %d,  size : %d\n", sndr->dest, sndr->channel,sndr->size);       
#endif
     return res;

          }
      }

      return  res;
      /*NOTREACHED*/
      break;

/*__________________________     end  PVM_1  Ioctls	*/

  }
  return 0;
}


/***************************************************/

static int
#ifndef WITH_KLD
pvmdriver_load(lkmtp, cmd)
#else
kld_pvmdriver_load(lkmtp, cmd)
#endif        
     struct lkm_table *lkmtp;
     int cmd;
{
#ifndef WITH_KLD
  struct lkm_table lkmt;
  struct lkm_any lkma;
#endif  
  int res;
  
  TRY("pvmdriver_load")

#ifdef DEBUG_HSL 
  log(LOG_DEBUG, "PVMDRIVER_load\n");
#endif

  PRINT_DEV("The PVM/MPC driver, LIP6/ASIM Laboratory, Olivier.GLUCK@lip6.fr\n");

#ifndef WITH_KLD
  /* Accept to be loaded only if hsl driver already loaded (if hsldriver is loaded, cmem driver is loaded */
  lkmt.private.lkm_any = &lkma;
  lkma.lkm_name = "hsldriver_mod";
  if (!lkmexists(&lkmt)) {
    log(LOG_ERR,
	"Can't load pvmdriver while hsldriver not loaded.\n");
    printf("Can't load pvmdriver while hsldriver not loaded.\n");
    return -1;
  }
#endif

  /*  Take a slot for PVM status */
  cm_status.size = (size_t) STATUS_SIZE_SPACE;
  strcpy(cm_status.name,"PVM/STAT");
  res = cmem_getmem(cm_status.size, cm_status.name);
  if (res >= 0) 
    {
      cm_status.size = round_page(cm_status.size);
      cm_status.kptr = (caddr_t) cmem_virtaddr(res);
      cm_status.pptr = cmem_physaddr(res);
      cm_status.slot = res;
#ifdef DEBUG_HSL
      log(LOG_DEBUG, "PVMDRIVER LOAD (size %d) (kptr %p) (pptr %p) (name %s) (slot %d)\n",
	  cm_status.size, cm_status.kptr, cm_status.pptr, cm_status.name, cm_status.slot);
#endif
    }
  else 
    {
      log(LOG_ERR, "Error with the return of cmem_getmem for status SLOT.\n");
      printf("Error with the return of cmem_getmem for status SLOT.\n");
      return res;
    }


    
  printf("\nThe PVMdriver is loaded with success !\n\t LIP6/ASIM Laboratory, Olivier.GLUCK@lip6.fr\n");


					/*kmana*/

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

/*******************************************************/
static int
#ifndef WITH_KLD
pvmdriver_unload(lkmtp, cmd)
#else
kld_pvmdriver_unload(lkmtp, cmd)
#endif        
     struct lkm_table *lkmtp;
     int cmd;
{
#ifdef DEBUG_HSL
  log(LOG_DEBUG, "PVMDRIVER_unload\n");
#endif

/* Free PVM CMEM SLOT for status */
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "UNLOAD PVM MODULE : free PVM status cmem slot (slot %d)\n", cm_status.slot);
#endif
    cmem_releasemem(cm_status.slot);
   

 printf("The PVM driver is unloaded.");

         /*kmana*/

/*
  if (!pci_unmap_int(tag)) {
    log(LOG_ERR, "Couldn't unmap interrupt\n");
    return 0;
  }

  log(LOG_DEBUG, "Interrupt correctly unmapped");
*/

  return 0;
}

/******************************************************/
static int
#ifndef WITH_KLD
pvmdriver_stat(lkmtp, cmd)
#else
kld_pvmdriver_stat(lkmtp, cmd)
#endif        
                  struct lkm_table *lkmtp;
                  int cmd;
{
		/* kmana, je n'en ai pas besoin pour le moment...*/
  return 0;
}

/*********************************************************/
/* 					 Point d'entree au driver					*/
/*********************************************************/
#ifndef WITH_KLD

int
pvminit(lkmtp, cmd, ver)
           struct lkm_table *lkmtp;
           int cmd;
           int ver;
{
#ifndef _SMP_SUPPORT
#define _module pvmdriver_module
  DISPATCH(lkmtp,
	   cmd,
	   ver,
	   pvmdriver_load,
	   pvmdriver_unload,
	   pvmdriver_stat)
#else
  DISPATCH(pvmdriver,
	   lkmtp,
	   cmd,
	   ver,
	   pvmdriver_load,
	   pvmdriver_unload,
	   pvmdriver_stat)
#endif
}

#else

static int
pvmdriver_load(mod, cmd, arg)
                  module_t mod;
                  modeventtype_t cmd;
                  void *arg;
{
  int err = 0;

  switch (cmd) {
  case MOD_LOAD:
    printf("Loaded pvm driver (KLD)\n");
    return kld_pvmdriver_load(NULL, 0);
    break;

  case MOD_UNLOAD:
    printf("Unloaded pvm driver (KLD).");
    return kld_pvmdriver_unload(NULL, 0);
    break;

  default: /* We only understand load/unload */
    err = EINVAL;
    break;
  }

  return err;
}

CDEV_MODULE(pvmdriver_mod, PVMMAJOR, cdev, pvmdriver_load, 0);

#endif

/*************************** CALLBACK FUNCTIONS *************************/

void Callbacksnd(int num)
{  
  * ((char *) (cm_status.kptr + num)) = (char) 1;
  cp_int_send ++;
  wakeup(cm_status.kptr + num);
/*  log(LOG_DEBUG,"Int SEND numero %d: numstatus = %d, STATUS = %d \n",cp_int_send,num,* ((char *) (cm_status.kptr + num)));*/
}
void Callbackrcv(int num)
{
  * ((char *) (cm_status.kptr + num)) = (char) 1;
  cp_int_recv ++;
/*  log(LOG_DEBUG,"Int RECV numero %d: numstatus = %d, STATUS = %d \n",cp_int_recv,num, * ((char *) (cm_status.kptr + num)));*/
}

/**************************************************************************/
/*                       PVM-SLRV  I/O  ctls _________end                 */
/**************************************************************************/

