
/* $Id: prot_garbcoll.c,v 1.1.1.1 1998/10/28 21:07:42 alex Exp $ */

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

int
main(int ac, char **av)
{
  int fd, res;

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  res = ioctl(fd, SLRVGARBAGECOLLECT);
  if (res < 0) {
    perror("ioctl SLRVGARBAGECOLLECT");
    exit(1);
  }

  close(fd);

  return 0;
}

