
/* $Id: dumpchannelstate.c,v 1.1.1.1 1998/10/28 21:07:42 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/data.h"
#include "../modules/driver.h"

int
main(ac, av)
     int ac;
     char **av;
{
  int res, dev;

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  res = ioctl(dev, DUMPCHANNELSTATE);
  if (res < 0) {
    perror("ioctl DUMPCHANNELSTATE");
    exit(1);
  }

  close(dev);

  return 0;
}
