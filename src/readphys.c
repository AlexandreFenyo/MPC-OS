
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

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
  physical_read_t pr;
  int count, i;
  int old_val;
  int diff;

  dev = open("/dev/hsl", O_RDWR);
  if (dev < 0) {
    perror("open");
    exit(1);
  }

  printf("0x");
  scanf("%x", (unsigned int *) &pr.src);

  if (ac == 1) {
    pr.len = 4;
    pr.data = (char *) buffer;

    res = ioctl(dev, HSLREADPHYS, &pr);
    if (res < 0) {
      perror("ioctl HSLREADPHYS");
      exit(1);
    }

    printf("0x%x -> 0x%x (%d)\n", (unsigned int) pr.src,
	   (unsigned int) buffer[0], (int) buffer[0]);
  } else {
    count = atoi(av[1]);

    pr.len = count;
    pr.data = (char *) buffer;

    res = ioctl(dev, HSLREADPHYS, &pr);
    if (res < 0) {
      perror("ioctl HSLREADPHYS");
      exit(1);
    }

    old_val = 0;
    for (i = 0; i < count; i++) {
      diff = ((int) ((u_char *) buffer)[i]) - old_val;
      printf("0x%x -> 0x%x (%d) (diff=%d)\n", i + (unsigned int) pr.src,
	     (unsigned int) ((u_char *) buffer)[i],
	     (int) ((u_char *) buffer)[i],
	     diff);
      old_val = ((u_char *) buffer)[i];
      if (diff != 1 && diff != -255)
	printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    }
  }

  close(dev);

  return 0;
}
