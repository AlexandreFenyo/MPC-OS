
#include <stdio.h>
#include <fcntl.h>
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
  unsigned int buffer[1024];
  physical_write_t pw;

  dev = open("/dev/hsl", O_RDWR);

  if (dev < 0) {
    perror("open");
    exit(1);
  }

  printf("0x");
  scanf("%x", (unsigned int *) &pw.dst);
  pw.len = 4;
  printf("data: 0x");
  scanf("%x", buffer);
  pw.data = (caddr_t) buffer;

  res = ioctl(dev, HSLWRITEPHYS, &pw);
  if (res < 0) {
    perror("ioctl HSLWRITEPHYS");
    exit(1);
  }

  return 0;
}
