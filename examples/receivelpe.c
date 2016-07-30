
/* $Id: receivelpe.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"

int
main(ac, av)
     int ac;
     char **av;
{
  int dev;
  char *chaine;
  char *p;
  int res;

  setvbuf(stdout, NULL, _IONBF, 0);

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  chaine = (char *) malloc(4096*2);
  if (!chaine) {
    perror("malloc");
    exit(1);
  }

  res = mlock(chaine, 4096*2);
  if (res < 0) {
    perror("mlock");
    exit(1);
  }

  p = chaine;
  if (ioctl(dev, HSLVIRT2PHYS, &p)) {
    perror("ioctl HSLVIRT2PHYS");
    exit(1);
  }

  printf("p = %p (%d bytes long)\n", p, 4096*2);

  for (ever) {
    printf("0x%x    \015", (u_int) chaine[0]);
    usleep(200000);
  }

  res = munlock(chaine, sizeof chaine);
  if (res < 0) {
    perror("munlock");
    exit(1);
  }

  close(dev);
}

