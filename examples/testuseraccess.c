
/* $Id: testuseraccess.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"
#include "../src/useraccess.h"

/* /usr/bin/cc -o testuseraccess testuseraccess.c useraccess.a */

int
main(int ac, char **av)
{
  opt_contig_mem_t *x;
  x = usrput_get_contig_mem(100);
  if (x) printf("ptr=0x%x\nsize=%d\nphys=0x%x\n",
	 (unsigned int) x->ptr,
	 x->size,
	 (unsigned int) x->phys);

  return 0;
}

