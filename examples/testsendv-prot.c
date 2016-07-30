
/* $Id: testsendv-prot.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

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
  int i;
  int count = 0;
  struct _virtualRegion vr;
  int size = 4096 * 2;

  if (ac >= 2)
    count = atoi(av[1]);

  if (ac >= 3)
    size = atoi(av[2]);
  vr.size  = size;

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  chaine = (char *) malloc(size);
  if (!chaine) {
    perror("malloc");
    exit(1);
  }

  for (i = 0; i < size; i++) chaine[i] = (i + 16) % 256;

  p = chaine + 1;
  if (ioctl(dev, HSLVIRT2PHYS, &p)) {
    perror("ioctl HSLVIRT2PHYS");
    exit(1);
  }

  printf("physical = %p\n", p);

  vr.start = chaine;

  for (; count >= 0; count--) {
    printf("sending (count=%d)\n", count);
    if (ioctl(dev, HSLTESTSLRVPROT, &vr)) {
      perror("ioctl HSLTESTSLRVPROT");
      exit(1);
    }
  }

  printf("waiting 30s\n");
  sleep(30);

  close(dev);

  return 0;
}

