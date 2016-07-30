
/* $Id: mmapdev.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"

int
main(int ac, char **av)
{
  caddr_t ptr;
  int fd, len;

  printf("0x");
  scanf("%x", (unsigned int *) &len);

  fd = open("/dev/kmem", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  ptr = mmap(NULL, 256, PROT_READ|PROT_WRITE, MAP_SHARED, fd, len);
  if (!ptr) {
    perror("mmap");
    exit(1);
  }

  printf("0x%x -> 0x%x\n",
	 (unsigned int) ptr,
	 (unsigned int) *((long *) ptr));
/* (*(long *) ptr)++; */

  munmap(ptr, 256);

  return 0;
}
