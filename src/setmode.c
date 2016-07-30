
/* $Id: setmode.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

void
usage(void)
{
  fprintf(stderr,
	  "usage: setmode [pnode] [value]\n");
  exit(0);
}

int
main(int ac, char **av)
{
  int fd;
  int res;
  set_mode_t mode;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  mode.node = 0;
  res = ioctl(fd, HSLSETMODEREAD, &mode);
  if (res < 0) {
    perror("ioctl HSLSETMODEREAD");
    exit(1);
  }

  switch (ac) {
  case 1:
    for (mode.node = 0; mode.node < mode.max_nodes; mode.node++) {
      res = ioctl(fd, HSLSETMODEREAD, &mode);
      if (res < 0) {
	perror("ioctl HSLSETMODEREAD");
	exit(1);
      }
      printf("mode(pnode=%d) <= %s\n",
	     mode.node,
	     mode.val ? "HSL" : "Ethernet");
    }
    usage();
    break;

  case 2:
    mode.node = atoi(av[1]);
    res = ioctl(fd, HSLSETMODEREAD, &mode);
    if (res < 0) {
      perror("ioctl HSLSETMODEREAD");
      exit(1);
    }
    printf("mode(pnode=%d) <= %s\n",
	   mode.node,
	   mode.val ? "HSL" : "Ethernet");
    break;

  case 3:
    mode.node = atoi(av[1]);
    mode.val = atoi(av[2]);
    res = ioctl(fd, HSLSETMODEWRITE, &mode);
    if (res < 0) {
      perror("ioctl HSLSETMODEWRITE");
      exit(1);
    }
    res = ioctl(fd, HSLSETMODEREAD, &mode);
    if (res < 0) {
      perror("ioctl HSLSETMODEREAD");
      exit(1);
    }
    printf("mode(pnode=%d) <= %s\n",
	   mode.node,
	   mode.val ? "HSL" : "Ethernet");
    break;

  default:
    usage();
    /* NOTREACHED */
    break;
  }

  close(fd);
  return 0;
}

