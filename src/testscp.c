
/* $Id: testscp.c,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"

#include "../src/useraccess.h"

#include "../modules/data.h"
#include "../modules/driver.h"

#define MAX(a,b) (((a)>(b))?(a):(b)) 
#define MIN(a,b) (((a)<(b))?(a):(b)) 

#define HSL2TTY_BUFSIZE 1024

int fd;

int p[2];

int opt_dist_node = -1;
int opt_microsec = 0;

void
handler_sigio(int val)
{
  write(p[1], "X", 1);
}

void
usage(void)
{
  printf("usage: testscp [-w microsec] -n node\n");
  exit(1);
}

int
main(int ac, char **av)
{
#if 0

  hsl2tty_t hsl2tty;
  char sendbuffer[HSL2TTY_BUFSIZE];
  char recvbuffer[HSL2TTY_BUFSIZE];
  fd_set readfds;
  int dest;
  int nbfd;
  char ch;
  int res;
  struct timeval waittime;

  while ((ch = getopt(ac, av, "w:n:")) != EOF)
    switch (ch) {
    case 'n':
      opt_dist_node = atoi(optarg);
      break;

    case 'w':
      opt_microsec = atoi(optarg);
      break;

    case '?':
    default:
      usage();
    }
  ac -= optind;
  av += optind;

  if (opt_dist_node == -1)
    usage();

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  res = pipe(p);
  if (res < 0) {
    perror("pipe");
    exit(1);
  }

  signal(SIGIO, handler_sigio);

  fprintf(stderr, "send a RECEIVE message\n");
  /* send a RECEIVE message to prepare the first reception */
  hsl2tty.addr = recvbuffer;
  hsl2tty.dest = opt_dist_node;
  hsl2tty.size = HSL2TTY_BUFSIZE;
  res = ioctl(fd, HSL2TTYRECV, &hsl2tty);
  if (res < 0) {
    perror("HSL2TTYRECV");
    exit(1);
  }

  /* Main loop */
  for (;;) {
    fprintf(stderr, "beginning of mainloop\n");
    
    /* Something ready to be read ? */
    FD_ZERO(&readfds);
    FD_SET(p[0], &readfds);
    waittime.tv_sec  = opt_microsec / 1000000;
    waittime.tv_usec = opt_microsec % 1000000;
    fprintf(stderr, "select()\n");
    nbfd = select(1 + p[0], &readfds, NULL, NULL, &waittime);
    if (nbfd < 0 && errno != EINTR) {
      perror("select");
      exit(1);
    }

    if (nbfd > 0 && FD_ISSET);

  }

  close(fd);

#endif

  return 0;
}

