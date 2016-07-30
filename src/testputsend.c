
/* $Id: testputsend.c,v 1.2 2000/02/23 18:11:26 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
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

int
main(int ac, char **av)
{
  opt_contig_mem_t *contig;
  int i;
  int res;
  char *s, *r;
  lpe_entry_t entry;
  char str[256];

  printf("(available flags = NOR | LMP | CM | LRM | LMI | SM | NOS | RESERVED | NONE)\n");

  printf("page length  = 0x");
  res = scanf("%x", &i);
  if (res != 1) {
    perror("scanf");
    exit(1);
  }
  entry.page_length = i;

  printf("routing part = 0x");
  res = scanf("%x", &i);
  if (res != 1) {
    perror("scanf");
    exit(1);
  }
  entry.routing_part = i;

  printf("flags        = ");
  do {
    s = fgets(str, sizeof(str), stdin);
    if (s == NULL) {
      perror("fgets");
      exit(1);
    }
  }
  while (strlen(s) < 2);

  entry.control = 0;

  while ((r = strtok(s, " \t,;|\n")) != NULL) {
    s = NULL;

    if (!strcmp(r, "NOR")) {
      entry.control |= NOR_MASK;
      continue;
    }

    if (!strcmp(r, "LMP")) {
      entry.control |= LMP_MASK;
      continue;
    }

    if (!strcmp(r, "CM")) {
      entry.control |= CM_MASK;
      continue;
    }

    if (!strcmp(r, "LRM")) {
      entry.control |= LRM_ENABLE_MASK;
      continue;
    }

    if (!strcmp(r, "IORP")) {
      entry.control |= IORP_MASK;
      continue;
    }

    if (!strcmp(r, "LMI")) {
      entry.control |= LMI_ENABLE_MASK;
      continue;
    }

    if (!strcmp(r, "SM")) {
      entry.control |= SM_MASK;
      continue;
    }

    if (!strcmp(r, "NOS")) {
      entry.control |= NOS_MASK;
      continue;
    }

    if (!strcmp(r, "RESERVED")) {
      entry.control |= RESERVED_MASK;
      continue;
    }

    if (!strcmp(r, "NONE"))
      continue;

    printf("[%s] => invalid flag\n", r);
    exit(1);
  };

  printf("PRSA         = 0x");
  res = scanf("%x", &i);
  if (res != 1) {
    perror("scanf");
    exit(1);
  }
  entry.PRSA = (caddr_t) i;

  printf("PLSA         = 0x");
  res = scanf("%x", &i);
  if (res != 1) {
    perror("scanf");
    exit(1);
  }
  entry.PLSA = (caddr_t) i;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  usrput_init();

  contig = usrput_get_contig_mem(SIZE);

  if (contig == NULL) {
    fprintf(stderr, "Error in usrput library\n");
    exit(1);
  }

  res = ioctl(fd, HSLTESTPUTSEND0, &entry);
  if (res < 0) {
    perror("ioctl HSLTESTPUTSEND0");
    exit(1);
  }

  usrput_end();

  close(fd);

  return 0;
}

