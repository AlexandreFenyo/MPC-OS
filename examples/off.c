
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include "../modules/driver.h"

main()
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
  conf.bus = i;

  printf("device   : ");
  conf.device = i;

  printf("func     : ");
  conf.func = i;

  printf("register : 0x");
  conf.reg = i;

  printf("data     : 0x");
  conf.data = u;

conf.bus = 0;
conf.device = 10;
conf.func = 0;
conf.reg = 0x48;
conf.data = 0x80000000;

  printf("[ %d %d %d %x %lx ]\n", conf.bus, conf.device, conf.func, conf.reg, conf.data);

  i = ioctl(fd, PCIWRITECONF, &conf);
  if (i < 0) {
    perror("ioctl");
    exit(1);
  }

  close(fd);
}
