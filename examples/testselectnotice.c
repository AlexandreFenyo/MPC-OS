
/* $Id: testselectnotice.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

int
main(int ac, char **av)
{
  fd_set readfds;
  char ch[256];
  int child;

  printf("fork ? [Y/N] ");
  fgets(ch, sizeof(ch), stdin);
  if (ch[0] == 'y' || ch[0] == 'Y') child = 1;
  else child = 0;

  if (child == 1)
    if (!fork()) {
      printf("child started\n");
      for (;;) {
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	select(1, &readfds, NULL, NULL, NULL);
	printf("child wakeup\n");

	if (child == 1)
	  if (FD_ISSET(0, &readfds)) read(0, ch, sizeof ch);
      }
      exit(0);
    }

    for (;;) {
      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
      select(1, &readfds, NULL, NULL, NULL);
      printf("process wakeup\n");

      if (child == 0)
	if (FD_ISSET(0, &readfds)) read(0, ch, sizeof ch);
    }

  printf("child exiting\n");
  return 0;
}
