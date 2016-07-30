
/* $Id: testlibmpc.c,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"
#include "mpcshare.h"

int main(int ac, char **av, char **ae)
{
  appclassname_t cn;
  appsubclassname_t sc;
  char ch[256];
  channel_t chan[2];
  u_short cluster;
  pnode_t pnode;
  int nclusters;
  int res;

  mpc_init();

  mpc_get_local_infos(&cluster, &pnode, &nclusters);

  printf("cluster=%d/pnode=%d/nclusters=%d\n", cluster, pnode, nclusters);
  printf("node_count[1st cluster]=%d\n", mpc_get_node_count(0));

  cn = make_appclass();
  printf("cn : 0x%lx\n", cn);

  mpc_spawn_task("sleep 10000; ls", atoi(av[1]), atoi(av[2]), cn);

  sc = make_subclass_prefnode(/*cluster*/ atoi(av[1]),
			      /* pnode */ atoi(av[2]),
			      /* value */ atoi(av[3]));

  res = mpc_get_channel(APPCLASS_INTERNET, sc, &chan[0], &chan[1], HSL_PROTO_SLRP_V);
  if (res) printf("mpc_get_channel() failed\n");
  else printf("chan0 = 0x%x - chan1 = 0x%x\n", (int) chan[0], (int) chan[1]);

  fgets(ch, sizeof(ch), stdin);

  delete_appclass(cn);

  mpc_close();
  return 0;
}

