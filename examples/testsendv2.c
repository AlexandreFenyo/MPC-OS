
/* $Id: testsendv2.c,v 1.2 2000/02/15 19:03:48 alex Exp $ */

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
  u_char *chaine;
  char *p;
  int i, res, compteur;

for (compteur=0; compteur < 10000; compteur++)
 {

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  chaine = (char *) malloc(20000);
  if (!chaine) {
    perror("malloc");
    exit(1);
  }

  for (i = 0; i < 20000; i++) chaine[i] = (i + 16) % 256;

  res = mlock(chaine, 20000);
  if (res < 0) {
    perror("mlock");
    exit(1);
  }

  p = chaine ;
  if (ioctl(dev, HSLVIRT2PHYS, &p)) {
    perror("ioctl HSLVIRT2PHYS");
    exit(1);
  }

  printf("physical = %p\n", p);

  p = chaine;

  if (ioctl(dev, HSLTEST2SLRV, &p)) {
    perror("ioctl HSLTEST2SLRV");
    exit(1);
  }

  /*  sleep(1); */

  res = munlock(chaine, 20000);
  if (res < 0) {
    perror("munlock");
    exit(1);
  }

  close(dev);
 }

  return 0;
}

