
/* $Id: rpctransport.x,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $ */

struct rpcentry {
	u_short routing_part;
	u_short page_length;
	u_long  control;
	u_long  PRSA;
	u_long  PLSA;
	u_char  data<>;
};

struct rpcconfig {
	u_char node;
	u_long contig_space;
};

struct sendrandompageparam {
	u_long wired_trash_data_addr;
	u_long v0;
};

program RPCTRANSPORT {
  version RPCTRANSPORTVERS {
	int SENDCONFIG(rpcconfig) = 1;
   	int LPEENTRY(rpcentry) = 2;
        int SENDRANDOMPAGE(sendrandompageparam) = 3;
  } = 2;
} = 101000;
