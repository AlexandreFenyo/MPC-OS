
/* $Id: virt2phys.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

#define SIZE 10000

int main()
{
  int fd;
  int res;
  char *vz;

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  printf("0x");
  scanf("%x", (unsigned int *) &vz);

  res = ioctl(fd, HSLVIRT2PHYS, &vz);
  if (res < 0) {
    perror("ioctl HSLVIRT2PHYS");
    exit(1);
  }
  printf("phys = 0x%x\n", (unsigned int) vz);

  close(fd);

  return 0;
}
