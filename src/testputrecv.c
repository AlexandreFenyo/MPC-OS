
/* $Id: testputrecv.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "../src/useraccess.h"

#include "../modules/data.h"
#include "../modules/driver.h"

#define SIZE 1024
#if SIZE > OPTIONAL_CONTIG_RAM_SIZE
#error SIZE too big
#endif

int fd;

void
end(int par)
{
  int res;

  fprintf(stderr, "\nclosing SAP\n");

  res = ioctl(fd, HSLTESTPUTRECV2, NULL);
  if (res < 0) {
    perror("ioctl HSLTESTPUTRECV2");
    exit(1);
  }

  exit(0);
}

int
main(int ac, char **av)
{
  opt_contig_mem_t *contig;
  int i;
  int res;
  mi_t mi;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  signal(SIGINT, end);

  usrput_init();

  contig = usrput_get_contig_mem(SIZE);

  if (contig == NULL) {
    fprintf(stderr, "Error in usrput library\n");
    exit(1);
  }

  printf("physical address = 0x%x\n",
	 (unsigned int) contig->phys);

  res = ioctl(fd, HSLTESTPUTRECV1, &mi);
  if (res < 0) {
    perror("ioctl HSLTESTPUTRECV1");
    exit(1);
  }

  printf("mi = 0x%x\n", (u_int) mi);

  for (i = 0; i < MIN(16, SIZE); i++)
    ((char *) contig->ptr)[i] = i;

  for (;;) {
    for (i = 0; i < MIN(16, SIZE); i++)
      printf("%x ", ((u_char *) contig->ptr)[i]);
    printf("\r");
  }

  usrput_end();

  close(fd);

  end(0);
}

