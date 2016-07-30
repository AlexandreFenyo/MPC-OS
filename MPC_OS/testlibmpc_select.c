
/* $Id: testlibmpc_select.c,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "mpc.h"
#include "mpcshare.h"

int main(int ac, char **av, char **ae)
{
  appsubclassname_t sc;
  char ch[256];
  channel_t chan[2];
  u_short cluster;
  pnode_t pnode, distnode;
  int nclusters;
  int i, res;
  fd_set readfds;
  mpc_chan_set readchs;
  struct timeval tv;
  int raw;

  if (ac > 1) raw = 1;
  else raw = 0;

  mpc_init();

  mpc_get_local_infos(&cluster, &pnode, &nclusters);

  distnode = (!pnode) ? 1 : 0;

  sc = make_subclass_prefnode(cluster, distnode, 0xAFAF);
  res = mpc_get_channel(APPCLASS_INTERNET, sc, &chan[0], &chan[1], HSL_PROTO_MDCP);
  if (res) printf("mpc_get_channel() failed\n");
  else if (!raw) printf("chan0 = 0x%x - chan1 = 0x%x\n", (int) chan[0], (int) chan[1]);

  for (;;) {
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    MPC_CHAN_ZERO(&readchs);
    MPC_CHAN_SET(distnode, chan[0], &readchs);
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    if (!raw) printf("mpc_select()\n");
    res = mpc_select(1, &readfds, NULL, NULL, &readchs, NULL, NULL, &tv);
    if (res < 0) {
      perror("mpc_select");
      exit(1);
    }
    if (!raw) printf("mpc_select() returned %d\n", res);

#if 0
    printf("readchs.max_index = %d\n", readchs.max_index);
    for (i = 0; i <= readchs.max_index; i++)
      printf("readchs: idx=%d is_set=%d\n", i, readchs.chan_set[i].is_set);
#endif

    if (FD_ISSET(0, &readfds) == TRUE) {
      if (!raw) printf("WE CAN READ ON STDIN\n");
      res = read(0, ch, sizeof ch);
      if (res < 0) {
	perror("read");
	exit(1);
      }
      if (!res) {
	printf("EOF!\n");
	exit(0);
      }
      if (res > 0) {
	res = mpc_write(distnode, chan[0], ch, res);
	if (res < 0) {
	  perror("write");
	  exit(1);
	}
      }
    }
    if (MPC_CHAN_ISSET(distnode, chan[0], &readchs) == TRUE) {
      if (!raw) printf("WE CAN READ ON THE CHANNEL\n");
      res = mpc_read(distnode, chan[0], ch, sizeof ch);
      if (res < 0) perror("mpc_read");
      if (!raw) printf("mpc_read() -> %d\n", res);
      if (!res) {
	printf("EOF!\n");
	exit(0);
      }
      if (res > 0) {
	if (raw) write(1, ch, res);
	else
	  for (i = 0; i < res; i++)
	    printf("ch[%d] = %#x (%c)\n", i, (int) ch[i], ch[i]);
      }
    }
  }

#if 0
  res = mpc_close_channel(APPCLASS_INTERNET, sc);
  printf("status = %d\n", res);
#endif

  mpc_close();
  return 0;
}

