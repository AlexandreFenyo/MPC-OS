
/* $Id: readpci.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

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
  scanf("%x", (unsigned int *) &i);
  conf.reg = i;

  printf("[ %d %d %d %x ]\n",
	 conf.bus,
	 conf.device,
	 conf.func,
	 (unsigned int) conf.reg);

  i = ioctl(fd, PCIREADCONF, &conf);
  if (i < 0) {
    perror("ioctl");
    exit(1);
  }

  printf("DATA = 0x%lx\n", conf.data);

  close(fd);

  return 0;
}
