
/* $Id: dumpPCIDDC.c,v 1.2 1998/12/18 17:57:41 alex Exp $ */

#define AVOID_LINK_ERROR

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/data.h"
#include "../modules/put.h"

int
main(int ac, char **av)
{
  int res;
  int fdmem;
  FILE *f;
  void *interrupt_table_ptr;
  void *interrupt_table;
  interrupt_table_t *itable;
  interrupt_table_t itheader;
  void *buf;
  size_t size;
  int i;

  if (ac != 2) {
    fprintf(stderr, "usage: %s LPE|LMI\n", av[0]);
    exit(1);
  }

  f = popen(strcmp(av[1], "LMI") ?
	    "../src/symaddr.sh hsldriver_mod.ko interrupt_table_LPE" :
	    "../src/symaddr.sh hsldriver_mod.ko interrupt_table_LMI", "r");
  if (!f) {
    perror("popen");
    exit(1);
  }
  res = fscanf(f, "%p", &interrupt_table_ptr);
  if (res != 1 || !interrupt_table_ptr) {
    fprintf(stderr, "%s: bad output from symaddr.sh", av[0]);
  }
  pclose(f);

  fdmem = open("/dev/kmem", O_RDONLY);
  if (fdmem < 0) {
    perror("open");
    exit(1);
  }

  buf = mmap(NULL,
	     sizeof(void *),
	     PROT_READ,
	     MAP_SHARED,
	     fdmem,
	     (int) interrupt_table_ptr);
  if (!buf) {
    perror("mmap");
    exit(1);
  }
  interrupt_table = *((void **) buf);
  munmap(buf, sizeof(void *));

  if (!interrupt_table)
    exit(1);

  itable = mmap(NULL,
		sizeof(interrupt_table_t),
		PROT_READ,
		MAP_SHARED,
		fdmem,
		(int) interrupt_table);
  if (!itable) {
    perror("mmap");
    exit(1);
  }
  itheader = *itable;
  size = sizeof(interrupt_table_t) +
    itable->maxentries * sizeof(interrupt_entry_t);
  munmap(itable, sizeof(interrupt_table_t));

  itable = mmap(NULL,
		size,
		PROT_READ,
		MAP_SHARED,
		fdmem,
		(int) interrupt_table);
  if (!itable) {
    perror("mmap");
    exit(1);
  }

  printf("maxentries: %d\n", itheader.maxentries);
  printf("nentries: %d\n\n", itheader.nentries);

  for (i = 0; i < itheader.nentries; i++) {
    printf(itable->interrupt[i].use_lpe ?
	   "%04x: idx=%04x fct=%08x lpe=T args(mi=%08x)\n" :
	   "%04x: idx=%04x fct=%08x lpe=F args(mi=%08x,d1=%08x,d2=%08x)\n",
	   i,
	   itable->interrupt[i].index,
	   (int) itable->interrupt[i].fct,
	   (int) itable->interrupt[i].args[1],
	   (int) itable->interrupt[i].args[2],
	   (int) itable->interrupt[i].args[3]);
  }

  munmap(itable, sizeof(interrupt_table_t));

  close(fdmem);
  return 0;
}

