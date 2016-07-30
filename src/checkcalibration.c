
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
  u_long old_data;

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

  conf.reg = 0x40;

  old_data = 0L;

  for (;;) {
    i = ioctl(fd, PCIREADCONF, &conf);
    if (i < 0) {
      perror("ioctl");
      exit(1);
    }

    if (old_data != (conf.data & 0xff)) {
      printf("calibration register changed: 0x%lx => 0x%lx\n",
	     old_data,
	     conf.data & 0xff);
      old_data = conf.data & 0xff;
    }
  }

  close(fd);

  return 0;
}
