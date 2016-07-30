
/* $Id: hslclient.c,v 1.4 1999/06/10 17:28:54 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <signal.h>
#include <machine/param.h>
#include <sys/param.h>

#include <sys/types.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/data.h"

#include "rpctransport_xdr.h"
#include "hslclient.h"

#include "../modules/driver.h"

static int argc;
static char **argv;

static int dev = -1;

static int put_initialized = 0;

static node_t node_number = -1;

static u_char (*routing_table)[258] = NULL;

#define HOSTNAME_MAXLENGTH 255

/*
 * Structure to handle information on nodes on the network :
 * - informations from the configuration file,
 * - structures used by the RPC to locate nodes we wish to talk to.
 */
typedef struct _nodeinfo {
  int in_use;
  char hostname[HOSTNAME_MAXLENGTH];
  u_long program;
  int version;
  CLIENT *client;
} nodeinfo_t;

static nodeinfo_t nodeinfo[MAX_NODES];

static int end_put __P((void));
static int end_slr __P((void));


/*
 ************************************************************
 * treat_sigint(sig) : handler for the SIGINT signal.
 * sig : the signal that occured.
 *
 * Soft exit of the process : closing PUT and SLR_P
 ************************************************************
 */

static void
treat_sigint(sig)
             int sig;
{
  if (put_initialized) {

    psignal(sig,
	    "Closing SLR_P interface");
    end_slr();

    psignal(sig,
	    "\nClosing PUT interface");
    end_put();

  } else psignal(sig,
		 "\nWARNING");

  if (dev != -1) close(dev);

  exit(1);
}


/*
 ************************************************************
 * usage() : display the command line options and exit
 ************************************************************
 */

static void
usage()
{
  printf("usage: %s -f config_file -d character_device [ -r routing_table ] [ -e error_rate ]\n",
	 argv[0]);
  exit(0);
}


/*
 ************************************************************
 * parse_config(config) : parse the configuration file.
 * config : the configuration file.
 *
 * exit on error.
 ************************************************************
 */

static void
parse_config(config)
             char *config;
{
  int i;
  FILE *f;
  char str[HOSTNAME_MAXLENGTH]; /* with this length, we can be sure that
				   in the following sscanf(), the buffer
				   for hostname will be long enough */
  nodeinfo_t tmp_ninfo;
  int tmp_node;

  printf("Parsing %s\n",
	 config);

  for (i = 0;
       i < MAX_NODES;
       i++)
    bzero(nodeinfo + i,
	  sizeof(nodeinfo_t));

  f = fopen(config,
	    "r");
  if (f == NULL) {
    perror("fopen");
    exit(1);
  }

  while (fgets(str,
	       sizeof str,
	       f)) {
    int res;
    bzero(&tmp_ninfo,
	  sizeof tmp_ninfo);

    res = sscanf(str,
		 "node %d hostname %s program %ld version %d\n",
		 &tmp_node,
		 tmp_ninfo.hostname,
		 &tmp_ninfo.program,
		 &tmp_ninfo.version);

#ifndef WITH_LOOPBACK
    if (res == 1) {
      if (tmp_node < 0 ||
	  tmp_node >= MAX_NODES) {
	fprintf(stderr,
		"node out of range\n");
	exit(1);
      }

      if (nodeinfo[tmp_node].in_use) {
	fprintf(stderr,
		"node in use\n");
	exit(1);
      }

      printf("my node number is %d\n",
	     tmp_node);
      node_number = tmp_node;
      nodeinfo[node_number].in_use = 1;
    }
#else
    if (res == 1) {
      res = sscanf(str,
		   "node %d loopback program %ld version %d\n",
		   &tmp_node,
		   &tmp_ninfo.program,
		   &tmp_ninfo.version);

      if (res != 3) {
	fprintf(stderr,
		"loopback: syntax error\n");
	exit(1);
      }

      strcpy(tmp_ninfo.hostname, "localhost");

      if (tmp_node < 0 ||
	  tmp_node >= MAX_NODES) {
	fprintf(stderr,
		"node out of range\n");
	exit(1);
      }

      if (nodeinfo[tmp_node].in_use) {
	fprintf(stderr,
		"node in use\n");
	exit(1);
      }

      printf("my node number is %d\n",
	     tmp_node);
      node_number = tmp_node;
      nodeinfo[tmp_node] = tmp_ninfo;
      nodeinfo[node_number].in_use = 1;
    }
#endif

    if (res == 4) {
      if (tmp_node < 0 ||
	  tmp_node >= MAX_NODES) {
	fprintf(stderr,
		"node out of range\n");
	exit(1);
      }

      if (nodeinfo[tmp_node].in_use) {
	fprintf(stderr,
		"node already used");
	exit(1);
      }

      nodeinfo[tmp_node] = tmp_ninfo;
      nodeinfo[tmp_node].in_use = 1;

      printf("foreign node %d\n  hostname : %s\n   program : %ld\n   version : %d\n",
	     tmp_node,
	     tmp_ninfo.hostname,
	     tmp_ninfo.program,
	     tmp_ninfo.version);

    }
  }

  if (ferror(f)) {
    perror("fgets");
    exit(1);
  }
}


/*
 ************************************************************
 * lookup_servers() : initializing RPC connections to the servers
 * on each node of the network.
 *
 * This function tries to connect to the different servers on the
 * network. Every two seconds, new connections are tried to all the
 * servers that were not started at the previous stage.
 * The function returns when it has been able to connect to all of
 * the servers.
 ************************************************************
 */

static void
lookup_servers()
{
  int i;
  int error;

  printf("\nTrying to connect to servers\n");

  do {
    error = 0;

    for (i = 0;
	 i < MAX_NODES;
	 i++) {
#ifndef WITH_LOOPBACK
      if (i == node_number ||
	  !nodeinfo[i].in_use ||
	  (nodeinfo[i].in_use && nodeinfo[i].client))
	continue;
#else
      if (!nodeinfo[i].in_use ||
	  (nodeinfo[i].in_use && nodeinfo[i].client))
	continue;
#endif

      nodeinfo[i].client = clnt_create(nodeinfo[i].hostname,
				       nodeinfo[i].program,
				       nodeinfo[i].version,
				       "tcp");

      if (nodeinfo[i].client)
	printf("connected to %s, program %ld, version %d\n",
	       nodeinfo[i].hostname,
	       nodeinfo[i].program,
	       nodeinfo[i].version);
      else {
	error = 1;
	fprintf(stderr,
		"error when trying to connect to %s, program %ld, version %d :\n",
		nodeinfo[i].hostname,
		nodeinfo[i].program,
		nodeinfo[i].version);

	clnt_pcreateerror(nodeinfo[i].hostname);
      }

    }

    if (error) {
      printf("waiting 2 seconds\n");
      sleep(2);
    }

  } while (error);

}


/*
 ************************************************************
 * load_routing_table(name) : map the routing table in memory.
 * name : the filename of the routing table in raw format.
 ************************************************************
 */

static void
load_routing_table(name)
                   char *name;
{
  int fd;

  fd = open(name, O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  routing_table = (u_char (*)[]) mmap(NULL,
				  sizeof(routing_table),
				  PROT_READ,
				  MAP_PRIVATE,
				  fd,
				  0);

  if (routing_table == NULL) {
    perror("mmap");
    exit(1);
  }

  close(fd);
}


/*
 ************************************************************
 * init_put(length, node) : initialization of the PUT interface.
 * length : length of the LPE the PUT layer is asked to allocate.
 * node : inform the PUT interface of our node number.
 *
 * return value : 0 if success,
 *                < 0 otherwise.
 ************************************************************
 */

static int
init_put(length, node, routing_table)
         int length;
         node_t node;
{
  int res;
  sigset_t sig;
  hsl_put_init_t init_s;

  init_s.length = length;
  init_s.node = node;
  init_s.routing_table = (u_char *) routing_table;

  /*
   * Make an atomic test&set sequence for put_initialized.
   */
  sig = sigmask(SIGINT);
  sigprocmask(SIG_BLOCK,
	      &sig,
	      NULL);
  fprintf(stdout,
	  "init_put: HSLPUTINIT\n");
  res = ioctl(dev,
	      HSLPUTINIT,
	      &init_s);
  if (res < 0)
    perror("init_put");
  else {
    /* initialiser ddslrp ici */
    put_initialized = 1;
  }
  sigprocmask(SIG_UNBLOCK,
	      &sig,
	      NULL);

  return res;
}


/*
 ************************************************************
 * init_slr() : initialization of the SLR/P interface.
 *
 * return value : NULL if error,
 *                physical address of the contig. memory area that
 *                has been allocated to handle the SLR/P protocol.
 ************************************************************
 */

static caddr_t
init_slr()
{
  int res;
  caddr_t param;

  res = ioctl(dev,
	      HSLSLRPINIT,
	      &param);
  if (res < 0) {
    perror("init_slr");
    return NULL;
  }
  return param;
}


/*
 ************************************************************
 * end_slr() : close the SLR/P interface.
 *
 * return value : 0 if success,
 *                < 0 otherwise.
 ************************************************************
 */

static int
end_slr()
{
  int res;

  res = ioctl(dev,
	      HSLSLRPEND);
  if (res < 0)
    perror("ioctl HSLSLRPEND");
  return res;
}


/*
 ************************************************************
 * end_put() : close the PUT interface.
 *
 * return value : 0 if success,
 *     (          < 0 otherwise.
 ************************************************************
 */
static int
end_put()
{
  int res;

  res = ioctl(dev,
	      HSLPUTEND);
  if (res < 0)
    perror("ioctl HSLPUTEND");
  return res;
}


/*
 ************************************************************
 * send_contig_space(contig_space) : send to every node on the
 * network the physical address of tables allocated by the SLR/P
 * interface for use by other nodes to transfer control
 * informations in the SLR/P protocol.
 * contig_space : physical address of the contig. space.
 ************************************************************
 */

static void
send_contig_space(contig_space)
                  caddr_t contig_space;
{
  node_t n;
  rpcconfig rpc_config;

  rpc_config.contig_space = (u_long) contig_space;
  for (n = 0;
       n < MAX_NODES;
       n++) {
    if (n == node_number ||
	!nodeinfo[n].in_use)
      continue;
    printf("Sending contig_space (%p) to server node %d\n",
	   contig_space,
	   n);
    rpc_config.node = node_number;
    sendconfig_2(&rpc_config,
		 nodeinfo[n].client);
  }
}


/*
 ************************************************************
 * main procedure :
 * - parse the configuration file,
 * - make RPC connections to all the nodes on the network,
 * - initialize PUT and SLR/P interfaces,
 * - send address of the physical memory allocated for use by other
 *   nodes to all of them,
 * - process the main loop of the job :
 *   - get an LPE entry,
 *   - allocate a buffer to handle the data before sending them to
 *     the destination node,
 *   - truncate the physical contig. space referenced in the LPE in
 *     several physical pages (of 4096 bytes or less for the first and
 *     the last) to load them one at a time in a part of the buffer,
 *   - send the buffer to the destination node,
 *   - simulate an interrupt because data have left the LPE,
 *   - loop.
 *
 * exit on error.
 ************************************************************
 */

int
main(ac, av)
     int ac;
     char **av;
{
  extern char *optarg;
  extern int optind;
  int c;
  int arg_f = 0;
  int arg_d = 0;
  int arg_r = 0;
  int arg_e = 0;
  char *arg_f_str = NULL;
  char *arg_d_str = NULL;
  char *arg_r_str = NULL;
  lpe_entry_t lpe_entry;
  rpcentry rpc_entry;
  int res;
  u_char *buffer = NULL;
  u_char *phys, *phys_max;
  physical_read_t pr;
  caddr_t contig_space;

  argc = ac;
  argv = av;

  /*
   * Catching interrupt signals to close the different interfaces
   * before exiting.
   */
  signal(SIGINT,
	 treat_sigint);

  /*
   * Parsing the configuration file.
   */
  while ((c = getopt(ac,
		     av,
		     "r:f:d:e:")) != EOF)
    switch (c) {
      /*
       * Routing table in raw format
       * (see chapter "Initialization" in RCube specification document).
       */
    case 'r':
      arg_r = 1;
      arg_r_str = optarg;
      break;

      /*
       * Configuration pathname.
       */
    case 'f':
      arg_f = 1;
      arg_f_str = optarg;
      break;

      /*
       * Device pathname.
       */
    case 'd':
      arg_d = 1;
      arg_d_str = optarg;
      break;

    case 'e':
      arg_e = atoi(optarg);
      /* Bad random number generator initialisation... */
      srandom(getpid());
      break;

    default:
      usage();
    }

  argc -= optind;
  if ((argc > 0) ||
      !arg_f ||
      !arg_d)
    usage();
  argv += optind;

  if (arg_r)
    load_routing_table(arg_r_str);

  if (arg_f)
    parse_config(arg_f_str);

  if (arg_d) {
    printf("\nOpening %s\n",
	   arg_d_str);
    dev = open(arg_d_str,
	       O_RDWR);
    if (dev < 0) {
      perror("open");
      exit(1);
    }
  }

  /*
   * Making RPC connections to the servers.
   */
  lookup_servers();

  /*
   * Initializing the PUT interface.
   */
  res = init_put(ETHERNET_LPE_SIZE,
		 node_number,
		 routing_table);
  if (res < 0 &&
      errno == EBUSY) { /* Last use of /dev/hsl was not clean... */
    printf("last use of layer PUT was not clean - Trying to correct...\n");
    end_put();
    res = init_put(ETHERNET_LPE_SIZE,
		   node_number,
		   routing_table);
  }
  if (res < 0) {
    perror("init_put");
    exit(1);
  }

  /*
   * Initializing the SLR/P interface.
   */
  contig_space = init_slr();
  if (!contig_space &&
      errno == EBUSY) { /* Last use was not clean... */
    printf("last use of layer SLR_P was not clean - Trying to correct...\n");
    end_slr();
    contig_space = init_slr();
  }
  if (!contig_space) {
    perror("init_slr");
    exit(1);
  }

  /*
   * Sending physical memory allocated to all the nodes on the network.
   */
  send_contig_space(contig_space);


  /*
   * THE MAIN LOOP.
   */

  for (ever) {

    /*
     * Get an LPE entry.
     */
    printf("Waiting for LPE entries\n");
    res = ioctl(dev,
		HSLGETLPE,
		&lpe_entry);
    if (res < 0) {
      perror("ioctl HSLGETLPE");
      exit(1);
    }
    if (!res && !lpe_entry.PRSA) {
      sendrandompageparam param;

      param.wired_trash_data_addr = (u_long) lpe_entry.PLSA;
      param.v0 = lpe_entry.control;

      printf("Warning : electromagnetic interaction detected around the wires.\n");

      sendrandompage_2(&param,
		       nodeinfo[(node_number == 0) ? 1 : 0].client);
    } else {

      printf("LPE entry got\n");

      if (!lpe_entry.page_length && !SM_ISSET(lpe_entry.control)) {
	fprintf(stderr,
		"zero length page !\n");
	exit(1);
      }

      if (SM_ISSET(lpe_entry.control) && lpe_entry.page_length != 0) {
	fprintf(stderr,
		"invalid size for a Short Message !\n");
	exit(1);
      }

      if (!SM_ISSET(lpe_entry.control)) {
	buffer = malloc(lpe_entry.page_length);
	if (!buffer) {
	  perror("malloc");
	  exit(1);
	}
      }

      /*
       * Truncate the physical contig. space referenced in the LPE in
       * several physical pages (of 4096 bytes or less for the first
       * and the last) to load them one at a time in a part of the buffer.
       */

      if (!SM_ISSET(lpe_entry.control)) {
	phys_max = lpe_entry.PLSA + lpe_entry.page_length;
	for (phys = lpe_entry.PLSA;
	     phys < phys_max;
	     phys = (u_char *) trunc_page((unsigned) (phys + PAGE_SIZE))) {
	  pr.src  = phys;
	  pr.len  = MIN((u_char *) trunc_page((unsigned) (phys + PAGE_SIZE)),
			phys_max) - phys;
	  pr.data = buffer + (phys - (u_char *) lpe_entry.PLSA);

	  printf("memcpy(virt:%p, phys:%p, %lu)\n",
		 pr.data,
		 pr.src,
		 pr.len);

	  /*
	   * Load the page in a part of the buffer.
	   */
	  res = ioctl(dev,
		      HSLREADPHYS,
		      &pr);
	  if (res < 0) {
	    perror("ioctl HSLREADPHYS");
	    exit(1);
	  }
	}
      }

      if (SM_ISSET(lpe_entry.control)) {
	rpc_entry.page_length = rpc_entry.data.data_len = 0;
	rpc_entry.data.data_val = NULL;
      } else {
	rpc_entry.page_length   = lpe_entry.page_length;
	rpc_entry.data.data_len = lpe_entry.page_length;
	rpc_entry.data.data_val = buffer;
      }

      rpc_entry.routing_part  = lpe_entry.routing_part;
      rpc_entry.control       = lpe_entry.control;
      rpc_entry.PRSA          = (u_long) lpe_entry.PRSA;
      rpc_entry.PLSA          = (u_long) lpe_entry.PLSA;

      {
	int i;

	for (i = 0; i < lpe_entry.page_length; i++)
	  printf("%d->%d ",
		 i,
		 rpc_entry.data.data_val[i]);
	printf("\n");
      }

      /*
       * Send the buffer to the destination node.
       */
      if (SM_ISSET(lpe_entry.control)) {
	printf("sending SM\n");
	printf("routing_part = %d / control = 0x%x / data1 = 0x%x / data2 = 0x%x\n",
	       rpc_entry.routing_part,
	       (u_int) rpc_entry.control,
	       (u_int) rpc_entry.PRSA,
	       (u_int) rpc_entry.PLSA);
      }

      if (nodeinfo[rpc_entry.routing_part].client == NULL) {
	fprintf(stderr,
		"null CLIENT, fatal error\n");
	fprintf(stderr,
		"routing_part = %d / control = 0x%x / PRSA = 0x%x / PLSA = 0x%x\n",
		rpc_entry.routing_part,
		(u_int) rpc_entry.control,
		(u_int) rpc_entry.PRSA,
		(u_int) rpc_entry.PLSA);
	exit(1);
      }

      if (arg_e && !(random() % arg_e))
	printf("simulating hardware fault -> page not transmitted\n");
      else
	lpeentry_2(&rpc_entry,
		   nodeinfo[rpc_entry.routing_part].client);

      if (!SM_ISSET(lpe_entry.control))
	free(buffer);

      /*
       * Simulate an interrupt because data have left the LPE.
       */
      res = ioctl(dev,
		  HSLFLUSHLPE);
      if (res < 0) {
	perror("ioctl HSLFLUSHLPE");
	exit(1);
      }
    }
  }

  return 0;
}

