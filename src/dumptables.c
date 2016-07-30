
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

  res = ioctl(dev, DUMPINTERNALTABLES);
  if (res < 0) {
    perror("ioctl DUMPINTERNALTABLES");
    exit(1);
  }

  close(dev);

  return 0;
}
