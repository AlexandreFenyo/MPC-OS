
/* $Id: testrecvv-prot.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

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
  boolean_t opt_bug = 0 /* FALSE */;
  struct _virtualRegion vr;
  int size = 4096 * 2;

  if (ac >= 4)
    opt_bug = 1 /* TRUE */;

  if (ac >= 3)
    size = atoi(av[2]);
  vr.size = size;

  if (ac >= 2)
    count = atoi(av[1]);

  setvbuf(stdout, NULL, _IONBF, 0);

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

  for (i = 0; i < size; i++) chaine[i] = 255;

  p = chaine;
  if (ioctl(dev, HSLVIRT2PHYS, &p)) {
    perror("ioctl HSLVIRT2PHYS");
    exit(1);
  }

  printf("physical = %p\n", p);

  p = chaine;

  if (opt_bug == 1 /* TRUE */)
    p = (caddr_t) main;

  vr.start = p;

  for (; count >= 0; count--) {
    printf("receiving (count=%d)\n", count);
    if (ioctl(dev, HSLTESTSLRV2PROT, &vr)) {
      perror("ioctl HSLTESTSLRV2PROT");
      exit(1);
    }
  }

  for (i = 0; i < 30; i++) {
    sleep(1);
    printf("%d : 0x%x            \015", 30 - i, chaine[0]);
  }

  close(dev);

  return 0;
}

