
/* $Id: testsend2wait.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

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
  int j;

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

  for (i = 0; i < 4096*2; i++) chaine[i] = (i + 16) % 256;

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

  for (j = 0; j < 100; j++) {
    char ch[100];
    chaine[0] = j;
    printf("%d [%d] - ", j, chaine[0]);
    fflush(stdout);

    scanf("%s", ch);

    if (ioctl(dev, HSLTESTSLR, &p)) {
      perror("ioctl HSLTESTSLR");
      exit(1);
    }

  }

  printf("waiting 30 sec\n");
  sleep(30);

  res = munlock(chaine, sizeof chaine);
  if (res < 0) {
    perror("munlock");
    exit(1);
  }

  close(dev);

  return 0;
}

