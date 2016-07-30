
/* $Id: testrecvv.c,v 1.2 2000/02/15 19:03:47 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"
#include "../modules/ddslrpp.h"

int
main(ac, av)
     int ac;
     char **av;
{
  int dev;
  char *chaine;
  char *p;
  int i, res;

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

  for (i = 0; i < 4096*2; i++) chaine[i] = 255;

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

  printf("physical = %p\n", p);

  p = chaine;

  if (ioctl(dev, HSLTESTSLRV2, &p)) {
    perror("ioctl HSLTESTSLRV2");
    exit(1);
  }

  for (i = 0; i < 30; i++) {
    sleep(1);
    printf("%d : 0x%x            \015", 30 - i, chaine[0]);
  }

  res = munlock(chaine, 4096 * 2);
  if (res < 0) {
    perror("munlock");
    exit(1);
  }

  close(dev);

  return 0;
}

