
/* $Id: testlibmpc2.c,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "mpcshare.h"

int main(int ac, char **av, char **ae)
{
  appsubclassname_t sc;
  int ret;

  mpc_init();

  sc = make_subclass_prefnode(/*cluster*/ atoi(av[1]),
			      /* pnode */ atoi(av[2]),
			      /* value */ atoi(av[3]));

  ret = mpc_close_channel(APPCLASS_INTERNET, sc);
  printf("status = %d\n", ret);
  mpc_close();
  return 0;
}

