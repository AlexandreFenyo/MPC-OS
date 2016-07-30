
/* $Id: addlpe.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

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
  lpe_entry_t entry;
  int res, i;

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

  printf("physical range = %p -> %p (%d bytes)\n",
	 (unsigned int *) p,
	 (unsigned int *) (p + 4096*2),
	 4096*2);

  entry.routing_part = 1;
  entry.page_length = /*4096*2*/ 4;
  printf("PRSA : 0x");
  scanf("%x", (unsigned int *) &entry.PRSA);
  /*entry.PRSA = (caddr_t) 1234;*/
  entry.PLSA = (caddr_t) p;
  entry.control = 2 | IORP_MASK;

  if (ioctl(dev, HSLADDLPE, &entry)) {
    perror("ioctl HSLADDLPE");
    exit(1);
  }

  res = munlock(chaine, sizeof chaine);
  if (res < 0) {
    perror("munlock");
    exit(1);
  }

  close(dev);

  return 0;
}

