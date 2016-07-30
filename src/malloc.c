
/* $Id */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int ac, char **av)
{
  char *ptr;

  ptr = (char *) malloc(30*1024*1024);
  if (ptr == NULL) {
    perror("malloc");
    exit(1);
  }

  bzero(ptr, 30*1024*1024);
  free(ptr);

  exit(0);
}
