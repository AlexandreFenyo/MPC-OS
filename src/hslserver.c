
/* $Id: hslserver.c,v 1.4 1999/06/10 17:28:54 alex Exp $ */

#include <sys/types.h>
#include <sys/errno.h>

#include "hslserver.h"

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"

#include "rpctransport_xdr.h"

#include <stdio.h>
#include <fcntl.h>
#include <rpc/rpc.h>

#include <stdlib.h>
#include <unistd.h>

static int dev = -1; /* Is device opened ? */

void
closedev(void)
{
  if (dev >= 0) {
    close(dev);
    dev = -1;
  }
}

void
opendevice(void)
{
  int ret;

  if (dev >= 0) return;

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  ret = atexit(closedev);
  if (ret) {
    perror("atexit");
    exit(1);
  }
}

#ifndef _SMP_SUPPORT
asm(".stabs \"___CTOR_LIST__\",22,0,0,_opendevice");
#endif

/*
 ************************************************************
 * lpeentry_2(entry) : treatment of data received from another node.
 * entry : LPE entry
 *
 * return value : 1 when success
 * exit on error
 ************************************************************
 */

int *
lpeentry_2(entry)
           rpcentry *entry;
{
  static int result; /* MUST BE STATIC */
  lpe_entry_t lpe;

  u_char *phys, *phys_max;
  physical_write_t pw;
  int res;

#ifdef _SMP_SUPPORT
  opendevice();
#endif

  /*
   * Fill in the LPE structure with the data just received.
   */
  lpe.routing_part = entry->routing_part;
  lpe.page_length  = entry->page_length;
  lpe.control      = entry->control;
  lpe.PRSA         = (caddr_t) entry->PRSA;
  lpe.PLSA         = (caddr_t) entry->PLSA;

  /*
   * Display the members of the LPE structure.
   */
  printf("LPE RECEIVED :\nrouting_part : %u\n", lpe.routing_part);
  printf("page_length : %u\n", lpe.page_length);
  printf("control : 0x%lx\n", lpe.control);
  printf("control : NOR  : %d\n", NOR_ISSET(lpe.control));
  printf("          LMP  : %d\n", LMP_ISSET(lpe.control));
  printf("          CM   : %d\n", CM_ISSET(lpe.control));
  printf("          IORP : %d\n", IORP_ISSET(lpe.control));
  printf("          NOS  : %d\n", NOS_ISSET(lpe.control));
  printf("          MI   : %lu\n", MI_PART(lpe.control));
  printf("PRSA : %p\n", lpe.PRSA);
  printf("PLSA : %p\n", lpe.PLSA);
  if (!SM_ISSET(lpe.control))
    printf("data : length = %d - first byte = 0x%x\n",
	   entry->data.data_len,
	   (int) *entry->data.data_val);

  /*
   * Display the data part.
   */
  if (!SM_ISSET(lpe.control)) {
    int i;

    for (i = 0; i < lpe.page_length; i++)
      printf("%d->%d ",
	     i,
	     entry->data.data_val[i]);
    printf("\n");
  }

  if (!SM_ISSET(lpe.control)) {
    /*
     * Fill in the physical memory with the data received.
     */

    phys_max = lpe.PRSA + lpe.page_length;

    /*
     * Fill in the physical memory one page at a time.
     */
    for (phys = lpe.PRSA;
	 phys < phys_max;
	 phys = (u_char *) trunc_page((unsigned) (phys + PAGE_SIZE))) {
      pw.dst  = phys;
      pw.len  = MIN((u_char *) trunc_page((unsigned) (phys + PAGE_SIZE)),
		    phys_max) - phys;
      pw.data = entry->data.data_val + (phys - (u_char *) lpe.PRSA);

      printf("memcpy(phys:%p, virt:%p, %lu)\n", pw.dst, pw.data, pw.len);

      /*
       * Call the driver /dev/hsl to write the data in kernel mode.
       */
      res = ioctl(dev, HSLWRITEPHYS, &pw);
      if (res < 0) {
	perror("ioctl HSLWRITEPHYS");
	exit(1);
      }
    }
  }

  /*
   * Ask the driver to simulate an interrupt, if needed.
   */
  res = ioctl(dev, HSLSIMINT, &lpe);
  if (res < 0) {
    perror("ioctl HSLSIMINT");
    exit(1);
  }

  /*
   * Return with success.
   */
  result = 1;
  return &result;
}


/*
 ************************************************************
 * sendconfig_2(entry) : treatment of configuration parameters
 * received from another node.
 * entry : configuration structure
 *
 * return value : 0 when success
 * exit on error
 ************************************************************
 */

int *
sendconfig_2(entry)
             rpcconfig *entry;
{
  static int result; /* MUST BE STATIC */
  slr_config_t config;
  int res;

#ifdef _SMP_SUPPORT
  opendevice();
#endif

  printf("Configuration received from %d : %p\n",
	 entry->node,
	 (caddr_t) entry->contig_space);

  /*
   * Fill in the config structure with the data just received.
   */
  config.node = entry->node;
  config.contig_space = (caddr_t) entry->contig_space;

  /*
   * Inform the driver with the configuration parameters just received.
   */
  res = ioctl(dev,
	      HSLSETCONFIG,
	      &config);
  if (res < 0) {
    perror("ioctl HSLSETCONFIG");
    exit(1);
  }

  /*
   * Return with success.
   */
  result = 0;
  return &result;
}


int *
sendrandompage_2(param)
     sendrandompageparam *param;
{
  static int result; /* MUST BE STATIC */
  sendrandompageinfo_t srpi;
  int res;

  printf("hslclient asks to send some pages again.\n");

  do {
    srpi.entry.page_length = 4;
    srpi.entry.routing_part = -1;
    srpi.entry.control = NOR_MASK | LMP_MASK | LMI_ENABLE_MASK;
    srpi.entry.PRSA = (caddr_t) param->wired_trash_data_addr;
    srpi.entry.PLSA = NULL;
    srpi.v0 = param->v0;

    res = ioctl(dev, HSLSENDRANDOMPAGE, &srpi);
    if (res < 0 && errno != EAGAIN) {
      perror("ioctl HSLSENDRANDOMPAGE");
      exit(1);
    }
  } while (res < 0);

  result = 0;
  return &result;
}
