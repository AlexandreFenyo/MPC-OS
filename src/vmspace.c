
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
  pid_t pid;

  if (ac != 2) {
    fprintf(stderr, "usage: %s pid\n", av[0]);
    exit(1);
  }

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  pid = atoi(av[1]);
  res = ioctl(fd, HSLVMINFO, &pid);
  if (res < 0) {
    perror("ioctl HSLVMINFO");
    exit(1);
  }

  close(fd);

  return 0;
}
