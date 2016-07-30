
/* $Id: hsl2tty.c,v 1.1.1.1 1998/10/28 21:07:42 alex Exp $ */

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

/* hsl2tty 1 /dev/ptyp3 */

/* #define HSL2TTY_BUFSIZE 1024 */
#define HSL2TTY_BUFSIZE 64000

#ifdef DEBUG_HSL
#undef DEBUG_HSL
#endif

int hsl, tty;

int p[2];

void handler_sigio(int val)
{
  write(p[1], "X", 1);
}

int
main(int ac, char **av)
{
  hsl2tty_t hsl2tty;
  int length;
  int res;
  char c;
  char sendbuffer[HSL2TTY_BUFSIZE];
  char recvbuffer[HSL2TTY_BUFSIZE];
  fd_set readfds;
  int dest;
  int nbfd;
  int val;
  int send_seq = 0;
  int tmpdest;

  res = pipe(p);
  if (res < 0) {
    perror("pipe");
    exit(1);
  }

  if (ac < 3) {
    fprintf(stderr, "invalid number of arguments\n");
    exit(1);
  }

  signal(SIGIO, handler_sigio);

  dest = atoi(av[1]);

  hsl = open("/dev/hsl", O_RDWR);
  if (hsl < 0) {
    perror("open hsl");
    exit(1);
  }

  tty = open(av[2], O_RDWR);
  if (tty < 0) {
    perror("open tty");
    exit(1);
  }

  tmpdest = dest;
  res = ioctl(hsl, HSL2TTYINIT, &tmpdest);
  if (res < 0) {
    perror("HSL2TTYINIT");
    exit(1);
  }

#ifdef DEBUG_HSL
  fprintf(stderr, "send a RECEIVE message\n");
#endif
  /* send a RECEIVE message to prepare the first reception */
  hsl2tty.addr = recvbuffer;
  hsl2tty.dest = dest;
  hsl2tty.size = HSL2TTY_BUFSIZE;
  res = ioctl(hsl, HSL2TTYRECV, &hsl2tty);
  if (res < 0) {
    perror("HSL2TTYRECV");
    exit(1);
  }

  /* Main loop */
  for (;;) {
#ifdef DEBUG_HSL
    fprintf(stderr, "beginning of mainloop\n");
#endif

    /* Something ready to be read ? */
    FD_ZERO(&readfds);
    FD_SET(p[0], &readfds);
    FD_SET(tty,  &readfds);
#ifdef DEBUG_HSL
    fprintf(stderr, "select()\n");
#endif
    nbfd = select(1 + MAX(tty, p[0]), &readfds, NULL, NULL, NULL);
    if (nbfd < 0 && errno != EINTR) {
      perror("select");
      exit(1);
    }

#ifdef DEBUG_HSL
    fprintf(stderr, "out of select()\n");
#endif

    if (nbfd > 0 && FD_ISSET(p[0], &readfds)) {
      /* Can read from HSL */
#ifdef DEBUG_HSL
      fprintf(stderr, "can read from HSL\n");
#endif

      /* Acknowledge this reception */
#ifdef DEBUG_HSL
      fprintf(stderr, "acknowledge the reception on the pipe\n");
#endif
      res = read(p[0], &c, 1);
      if (res != 1) {
	perror("read pipe");
	exit(1);
      }

      /* Forward data to the TTY */
      length = MIN(HSL2TTY_BUFSIZE - 4, *(int *) recvbuffer);
#ifdef DEBUG_HSL
      fprintf(stderr, "forward data to the TTY : length=4+%d\n",
	      length);
#endif
      res = write(tty, recvbuffer + 4, length);
      if (res < 0) {
	perror("write tty");
	exit(1);
      }
      if (res != length) {
	fprintf(stderr, "write tty: res < len!\n");
	exit(1);
      }

      /* Prepare a new reception */
#ifdef DEBUG_HSL
      fprintf(stderr, "send a RECEIVE message\n");
#endif
      hsl2tty.addr = recvbuffer;
      hsl2tty.dest = dest;
      hsl2tty.size = HSL2TTY_BUFSIZE;
      res = ioctl(hsl, HSL2TTYRECV, &hsl2tty);
      if (res < 0) {
	perror("HSL2TTYRECV");
	exit(1);
      }
    }

    if (nbfd > 0  && FD_ISSET(tty, &readfds)) {
      /* Can read from the TTY */
#ifdef DEBUG_HSL
      fprintf(stderr, "can read from TTY\n");
#endif

      /*
       * Can we send on HSL ?
       * Note that without this test, there is an interblocking race condition,
       * see hsl2tty.c.bad for more informations.
       */
      val = dest;
#ifdef DEBUG_HSL
      fprintf(stderr, "can send on HSL seq=0x%x ?\n", send_seq);
#endif
      res = ioctl(hsl, HSL2TTYCANSEND, &val);
      if (res != 0) {
	perror("HSL2TTYCANSEND");
	exit(1);
      }
      if (val) {
	/* Yes, we can write on HSL */
#ifdef DEBUG_HSL
	fprintf(stderr, "Yes, can send on HSL\n");
#endif

	/* Read data */
#ifdef DEBUG_HSL
	fprintf(stderr, "read data from TTY\n");
#endif
	res = read(tty, sendbuffer + 4, HSL2TTY_BUFSIZE - 4);
	if (res < 0) {
	  perror("read tty");
	  exit(1);
	}
	if (res == 0) {
	  fprintf(stderr, "tty EOF!\n");
	  exit(1);
	}
	length = res;

	/* fprintf(stderr, "[%s]\n", (char *) (sendbuffer + 4)); */

	/* Forward data to HSL */
	hsl2tty.addr = sendbuffer;
	hsl2tty.dest = dest;
	hsl2tty.size = 4 + length;
	*(int *) sendbuffer = length;
#ifdef DEBUG_HSL
	fprintf(stderr, "forward data to HSL (length=%d+4, seq=0x%x)\n",
		length, send_seq);
#endif
	res = ioctl(hsl, HSL2TTYSEND, &hsl2tty);
	if (res < 0) {
	  perror("HSL2TTYSEND");
	  exit(1);
	}
#ifdef DEBUG_HSL
	fprintf(stderr, "End of forwarding\n");
#endif
	send_seq++;
	if (hsl2tty.size != 4 + length) {
	  fprintf(stderr, "invalid sent size\n");
	  exit(1);
	}
      } else {
	/* No, we can't write on HSL without blocking */

#ifdef DEBUG_HSL
	fprintf(stderr, "can't write on HSL, retrying...\n");
#endif
	/* Avoid active wait... */
	/* usleep(1000000); */
      }
    }
  }

  close(tty);
  close(hsl);

  return 0;
}

