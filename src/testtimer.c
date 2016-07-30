
/* $Id: testtimer.c,v 1.2 2000/02/23 18:11:27 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../MPC_OS/mpcshare.h"

#include "../src/useraccess.h"

#include "../modules/data.h"
#include "../modules/driver.h"

int fd;

int
main(int ac, char **av)
{
  int res;
  int limit;
  struct timeval tv;

  if (ac != 2) {
    fprintf(stderr, "usage: %s counter_limit\n", av[0]);
    exit(1);
  }

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  limit = atoi(av[1]);

  gettimeofday(&tv, NULL);
  printf("seconds: %ld ; microsec: %ld\n", tv.tv_sec, tv.tv_usec);

  res = ioctl(fd, HSLTESTTIMER, &limit);
  if (res < 0) {
    perror("ioctl HSLTESTTIMER");
    exit(1);
  }

  gettimeofday(&tv, NULL);
  printf("seconds: %ld ; microsec: %ld\n", tv.tv_sec, tv.tv_usec);

  close(fd);

  return 0;
}

