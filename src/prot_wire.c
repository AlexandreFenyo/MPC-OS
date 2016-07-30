
/* $Id: prot_wire.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

#define SIZE1 100

int
main(int ac, char **av)
{
  int fd, res;
  struct _virtualRegion vr;

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  printf("waiting 5 sec\n");
  sleep(5);

  if (ac == 1) {

    vr.start = (caddr_t) main;
    vr.size  = 16;

    printf("wiring\n");
    res = ioctl(fd, HSLPROTECTWIRE, &vr);
    if (res < 0) {
      perror("ioctl HSLPROTECTWIRE");
      exit(1);
    }
  }

  if (ac == 2) {
    char *t1, *t2;

    t1 = malloc(SIZE1);
    if (t1 == NULL) {
      perror("malloc");
      exit(1);
    }

    vr.start = t1;
    vr.size  = SIZE1;

    printf("wiring %d bytes at 0x%x\n", (u_int) vr.size, (u_int) vr.start);

    res = ioctl(fd, HSLPROTECTWIRE, &vr);
    if (res < 0) {
      perror("ioctl HSLPROTECTWIRE");
      exit(1);
    }

    sleep(5);

    t2 = malloc(atoi(av[1]));

    if (t2 == NULL) {
      perror("malloc");
      exit(1);
    }

    vr.start = t2;
    vr.size  = atoi(av[1]);

    printf("wiring %d bytes at 0x%x\n", (u_int) vr.size, (u_int) vr.start);

    res = ioctl(fd, HSLPROTECTWIRE, &vr);
    if (res < 0) {
      perror("ioctl HSLPROTECTWIRE");
      exit(1);
    }
  }

  printf("waiting 5 sec\n");
  sleep(5);

  close(fd);

  return 0;
}

