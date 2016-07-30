
/* $Id: getevents.c,v 1.1.1.1 1998/10/28 21:07:42 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

int
main(ac, av)
     int    ac;
     char **av;
{
  int fd, res;
  int first_tv_sec = 0;
  trace_event_t event;
  int old_tv_sec = 0;
  int old_tv_usec = 0;
  u_int diff;

  setvbuf(stdout, NULL, _IONBF, 0);

  printf("starting\n");

  do {
    fd = open("/dev/hsl", O_RDONLY);
    if (fd < 0) sleep(1);
  }
  while (fd < 0);

  for (;;) {
    res = ioctl(fd, HSLGETEVENT, &event);
    if (res) {
      perror("ioctl HSLGETEVENT");
      exit(1);
    }

    if (!first_tv_sec) {
      old_tv_sec = first_tv_sec = event.tv.tv_sec;
      old_tv_usec = event.tv.tv_usec;
    }

    diff = 1000000 * event.tv.tv_sec + event.tv.tv_usec -
      (1000000 * old_tv_sec + old_tv_usec);
    if (diff < 10000000 && diff >= 0)
      printf("%d.%06d [%07d] index %d type %d entry_type %02d message",
	     (int) (event.tv.tv_sec - first_tv_sec), (int) event.tv.tv_usec,
	     diff,
	     event.index, event.type, event.entry_type);
    else
      printf("%d.%06d [-------] index %d type %d entry_type %02d message",
	     (int) (event.tv.tv_sec - first_tv_sec), (int) event.tv.tv_usec,
	     event.index, event.type, event.entry_type);
    if (event.type == TRACE_STR)
      printf(" %s\n", event.trace.str);
    else {
      printf(" %d", event.trace.var.pending_send_entry);
      printf(" %d", event.trace.var.pending_receive_entry);
      printf(" %d", event.trace.var.send_phys_addr);
      printf(" %d", event.trace.var.copy_phys_addr);
      printf(" %d", event.trace.var.channel_0_in);
      printf(" %d", event.trace.var.channel_0_out);
      printf(" %d", event.trace.var.sequence_0_in);
      printf(" %d", event.trace.var.sequence_0_out);
      printf(" %d", event.trace.var.channel_1_in);
      printf(" %d", event.trace.var.channel_1_out);
      printf(" %d", event.trace.var.sequence_1_in);
      printf(" %d", event.trace.var.sequence_1_out);
      printf("\n");
    }

    old_tv_sec  = event.tv.tv_sec;
    old_tv_usec = event.tv.tv_usec;
  }

  close(fd);
  return 0;
}
