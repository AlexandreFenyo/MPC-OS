
/* $Id: writepci.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

int
main(int ac, char **av)
{
  int fd;
  int i;
  unsigned long u;
  mpc_pci_conf_t conf;

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  printf("bus      : ");
  scanf("%d", &i);
  conf.bus = i;

  printf("device   : ");
  scanf("%d", &i);
  conf.device = i;

  printf("func     : ");
  scanf("%d", &i);
  conf.func = i;

  printf("register : 0x");
  scanf("%x", &i);
  conf.reg = i;

  printf("data     : 0x");
  scanf("%lx", &u);
  conf.data = u;


  printf("[ %d %d %d %x %lx ]\n",
	 conf.bus,
	 conf.device,
	 conf.func,
	 (unsigned int) conf.reg,
	 (unsigned long) conf.data);

  i = ioctl(fd, PCIWRITECONF, &conf);
  if (i < 0) {
    perror("ioctl");
    exit(1);
  }

  close(fd);

  return 0;
}
