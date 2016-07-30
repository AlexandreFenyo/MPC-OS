
/* $Id: vmtest.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <unistd.h>

int x;
int y = 1;

int
main(int ac, char **av, char **ae)
{
  int a;

  printf("start\n");
  printf("init.data: %p notinit.data: %p stack: %p param: %p env: %p ae: %p\n",
	 &y, &x, &a, av[0], ae[0], ae);

  sleep(10000);

  printf("finish\n");

  return 0;
}
