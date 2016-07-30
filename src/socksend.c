
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/uio.h>
#include <unistd.h>

#include <string.h>
#include <sys/fcntl.h>

int
main(int ac, char **av)
{
  struct sockaddr saddr;
  int s;
  int res;

  if (ac < 2) {
    fprintf(stderr, "usage: %s sockname\n", av[0]);
    exit(1);
  }

  bzero(&saddr, sizeof saddr);
  saddr.sa_family = AF_UNIX;
  saddr.sa_len = sizeof saddr;
  strncpy(saddr.sa_data, av[1], sizeof(saddr.sa_data));

  s = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket");
    exit(1);
  }

  res = connect(s, &saddr, sizeof saddr);
  if (res < 0) {
    perror("connect");
    exit(1);
  }

  res = write(s, "TESTING", 7);

  if (res < 0) {
    perror("write");
    exit(1);
  }

  sleep(2);

  res = shutdown(s, 2);
  if (res < 0) {
    perror("shutdown");
    exit(1);
  }

  close(s);

  return 0;
}
