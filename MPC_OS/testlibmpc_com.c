
/* $Id: testlibmpc_com.c,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <stdio.h>
#include <stdlib.h>

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

  mpc_init();

  mpc_get_local_infos(&cluster, &pnode, &nclusters);

  distnode = (!pnode) ? 1 : 0;

  sc = make_subclass_prefnode(cluster, distnode, 0xAFB0);

  res = mpc_get_channel(APPCLASS_INTERNET, sc, &chan[0], &chan[1], HSL_PROTO_MDCP);
  if (res) printf("mpc_get_channel() failed\n");
  else printf("chan0 = 0x%x - chan1 = 0x%x\n", (int) chan[0], (int) chan[1]);

  fgets(ch, sizeof(ch), stdin);

  res = mpc_read(distnode, chan[0], ch, sizeof ch);
  if (res < 0) perror("mpc_read");

  printf("mpc_read() -> %d\n", res);

  if (res > 0)
    for (i = 0; i < res; i++)
      printf("ch[%d] = %#x (%c)\n", i, (int) ch[i], ch[i]);

#if 0
  res = mpc_close_channel(APPCLASS_INTERNET, sc);
  printf("status = %d\n", res);
#endif

  mpc_close();
  return 0;
}

