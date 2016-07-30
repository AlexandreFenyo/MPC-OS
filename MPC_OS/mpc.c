
/* $Id: mpc.c,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $ */

#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/uio.h>
#include <unistd.h>

#include <string.h>
#include <sys/fcntl.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>

#include <sys/stat.h>
#include <sys/ioccom.h>

#include <sys/errno.h>

#include "mpcshare.h"
#include "../modules/data.h"
#include "../modules/driver.h"

#include "mpc.h"

/* __progname is set by crt0.o */
extern char *__progname;

static int sock = -1;
static int dev  = -1;
FILE *mpc_stderr;

int /*bool*/ dont_exit = 0;

static int mpcread  __P((char *buf, int size));
static int mpcwrite __P((char *buf, int size));
static void command __P((unsigned long io, char *com));

void
mpc_init(void)
{
  struct sockaddr saddr;
  int res;
  pid_t pid;
  int key;
  static char name[] = "/tmp/mpcauth.XXXX";
  char ch[256];
  mpcmsg_myname_t msgname;

  if (getenv("DUMP_FILE")) {
    sprintf(ch, "/tmp/libmpc.%s", __progname);
    mpc_stderr = fopen(ch, "w");
    if (mpc_stderr == NULL) mpc_stderr = stderr;
  } else {
    mpc_stderr = stderr;
  }
  setvbuf(mpc_stderr, NULL, _IONBF, 0);

  if (getenv("DONT_EXIT")) dont_exit = TRUE;

  if (getenv("MPCSOCK"))
    strncpy(saddr.sa_data, getenv("MPCSOCK"), sizeof(saddr.sa_data));
  else
    strncpy(saddr.sa_data, "/tmp/mpc-100", sizeof(saddr.sa_data));

  sock = socket(PF_LOCAL, SOCK_STREAM, 0);
  if (sock < 0) {
    MPC_PERROR("MPCLIB socket");
    if (!dont_exit) _exit(1);
  }

  res = connect(sock, &saddr, sizeof saddr);
  if (res < 0) {
    MPC_PERROR("MPCLIB connect");
    if (!dont_exit) _exit(1);
  }

  if (getenv("MPCKEY"))
    key = atoi(getenv("MPCKEY"));
  else
    key = 100;

  res = read(sock, name, 17);
  if (res < 0) {
    MPC_PERROR("MPCLIB write");
    if (!dont_exit)_exit(1);
  }

  res = open(name, O_RDWR | O_CREAT, 0600);
  if (res < 0) {
    MPC_PERROR("MPCLIB open");
    if (!dont_exit) _exit(1);
  }
  close(res);

  pid = getpid();
  res = write(sock, &pid, sizeof pid);
  if (res < 0) {
    MPC_PERROR("MPCLIB write");
    if (!dont_exit) _exit(1);
  }

  strncpy(msgname.name, __progname, sizeof(msgname.name));
  msgname.name[sizeof(msgname.name) - 1] = 0;
  command(MPCMSGMYNAME, (caddr_t) &msgname);

  dev = open(HSL_DEVICE, O_RDWR);
  if (dev < 0) {
    MPC_PERROR("MPCLIB open device");
    if (!dont_exit)_exit(1);
  }
}

void
mpc_close(void)
{
  int res;

  if (sock >= 0) {
    res = shutdown(sock, 2);
    if (res < 0) {
      MPC_PERROR("MPCLIB shutdown");
      if (!dont_exit) _exit(1);
    }
  }

  close(dev);
  dev = -1;

  close(sock);
  sock = -1;
}

appsubclassname_t
make_subclass_prefnode(u_short cluster, pnode_t pnode, primary_t value)
{
  appsubclassname_t subc;

  subc.controlled.prefnode_cluster = cluster;
  subc.controlled.prefnode_pnode   = pnode;
  subc.controlled.value            = value;

  return subc;
}

appsubclassname_t
make_subclass_raw(primary_t value)
{
  appsubclassname_t subc;

  subc.is_raw        = 0xFFFF;
  subc.raw.primary   = value;
  subc.raw.secondary = 0;

  return subc;
}

appclassname_t
make_appclass(void)
{
  appclassname_t cn;

  command(MPCMSGMAKEAPPCLASS, (caddr_t) &cn);
  return cn;
}

int
delete_appclass(appclassname_t cn)
{
  /* NOT YET IMPLEMENTED */
  return 0;
}

static int
mpcwrite(char *buf, int size)
{
  char *p;
  int count;
  int res;

  p = buf;
  count = 0;

  while (count != size) {
    res = write(sock, p, size);

    if (res < 0 && res != EINTR) {
      MPC_PERROR("mpcwrite");
      if (!dont_exit) _exit(92);
      else return -1;
    }

    if (res < 0 && res == EINTR) {
      MPC_PERROR("mpcwrite");
      continue;
    }

    if (res == 0) {
      fprintf(mpc_stderr, "mpcwrite return 0 !\n");
      continue;
    }

    /* Here, res > 0 */
    count += res;
    p     += res;
  }

  return size;
}

static int
mpcread(char *buf, int size)
{
  char *p;
  int count;
  int res;

  p = buf;
  count = 0;

  while (count != size) {
    res = read(sock, p, size);

    if (res < 0 && res != EINTR) {
      MPC_PERROR("MPCLIB mpcread");
      if (!dont_exit) _exit(92);
      else return -1;
    }

    if (res < 0 && res == EINTR) {
      MPC_PERROR("MPCLIB mpcread");
      continue;
    }

    if (res == 0) {
      fprintf(mpc_stderr, "MPCLIB EOF!\n");
      if (!dont_exit) _exit(92);
      else return -1;
    }

    /* Here, res > 0 */
    count += res;
    p     += res;
  }

  return size;
}

static void
command(unsigned long io, char *com)
{
  mpcwrite((char *) &io, sizeof io);
  if (io & IOC_IN) mpcwrite(com, IOCPARM_LEN(io));
  if (io & IOC_OUT) mpcread(com, IOCPARM_LEN(io));
}

int
mpc_get_channel(appclassname_t mainclass, appsubclassname_t subclass,
		channel_t *ret_channel_pair_0, channel_t *ret_channel_pair_1,
		int protocol)
{
  mpcmsg_get_channel_t msg;

  msg.mainclass = mainclass;
  msg.subclass  = subclass;
  msg.protocol  = protocol;
  if (subclass.is_raw == 0xFFFF) {
    /* NOT YET IMPLEMENTED */
    return -1;
  } else {
    command(MPCMSGGETCHANNEL, (caddr_t) &msg);
    if (msg.status != 0 && msg.status != ENOMEM) {
      fprintf(mpc_stderr, "MPCLIB get_channel - status=%d\n", msg.status);
      _exit(91);
    }
    if (msg.status == ENOMEM) return -1;

    *ret_channel_pair_0 = msg.channel_pair_0;
    *ret_channel_pair_1 = msg.channel_pair_1;
    return 0;
  }
}

int
mpc_close_channel(appclassname_t mainclass, appsubclassname_t subclass)
{
  mpcmsg_close_channel_t msg;

  msg.mainclass = mainclass;
  msg.subclass  = subclass;
  if (subclass.is_raw == 0xFFFF) {
    /* NOT YET IMPLEMENTED */
    return -1;
  } else {
    command(MPCMSGCLOSECHANNEL, (caddr_t) &msg);
    if (msg.status != 0) {
      fprintf(mpc_stderr, "MPCLIB close_channel - status=%d\n", msg.status);
      _exit(91);
    }

    return 0;
  }
}

int
mpc_get_local_infos(u_short *ret_cluster, pnode_t *ret_pnode, int *ret_nclusters)
{
  mpcmsg_get_local_infos_t msg;

  command(MPCMSGGETLOCALINFOS, (caddr_t) &msg);

  *ret_cluster   = msg.cluster;
  *ret_pnode     = msg.pnode;
  *ret_nclusters = msg.nclusters;

  return 0;
}

int
mpc_get_node_count(int cluster)
{
  mpcmsg_get_node_count_t msg;

  msg.cluster = cluster;

  command(MPCMSGGETNODECOUNT, (caddr_t) &msg);
  return msg.node_count;
}

int
mpc_spawn_task(char *cmdline, u_short cluster, pnode_t pnode, appclassname_t cn)
{
  mpcmsg_spawn_task_t msg;

  if (strlen(cmdline) >= CMDLINE_SIZE) return -1;
  strcpy(msg.cmdline, cmdline);
  msg.cluster   = cluster;
  msg.pnode     = pnode;
  msg.mainclass = cn;

  command(MPCMSGSPAWNTASK, (caddr_t) &msg);
  if (msg.status != 0) {
    fprintf(mpc_stderr, "MPCLIB spawn_task\n");
    _exit(91);
  }

  return 0;
}

int
mpc_read(pnode_t dest, channel_t channel, void *buf, size_t nbytes)
{
  int res;
  libmpc_read_t params;

  params.node = dest;
  params.channel = channel;
  params.buf = buf;
  params.nbytes = nbytes;

  if (dev >= 0) {
    res = ioctl(dev, LIBMPCREAD, &params);
    if (res < 0) return res;
    return params.nbytes;
  }

  return 0;
}

int
mpc_write(pnode_t dest, channel_t channel, const void *buf, size_t nbytes)
{
  int res;
  libmpc_write_t params;

  params.node = dest;
  params.channel = channel;
  params.buf = buf;
  params.nbytes = nbytes;

  if (dev >= 0) {
    res = ioctl(dev, LIBMPCWRITE, &params);
    if (res < 0) return res;
    return params.nbytes;
  }

  return 0;
}

void
MPC_CHAN_SET(pnode_t dest, channel_t chan, mpc_chan_set *mpcchanset)
{
  int i;

  if (mpcchanset->max_index == -1) {
    mpcchanset->max_index = 0;
    mpcchanset->chan_set[0].is_set = TRUE;
    mpcchanset->chan_set[0].dest = dest;
    mpcchanset->chan_set[0].channel = chan;
    return;
  }

  for (i = 0; i < mpcchanset->max_index; i++)
    if (mpcchanset->chan_set[i].is_set == FALSE) {
      mpcchanset->chan_set[i].is_set  = TRUE;
      mpcchanset->chan_set[i].dest    = dest;
      mpcchanset->chan_set[i].channel = chan;
      return;
    }

  if (mpcchanset->max_index == MAX_CHAN_PROC - 1) {
    fprintf(mpc_stderr, "MPC_CHAN_SET: mpc_chan_set full\n");
    _exit(91);
  }

  mpcchanset->max_index++;
  mpcchanset->chan_set[mpcchanset->max_index].is_set  = TRUE;
  mpcchanset->chan_set[mpcchanset->max_index].dest    = dest;
  mpcchanset->chan_set[mpcchanset->max_index].channel = chan;
}

void
MPC_CHAN_CLR(pnode_t dest, channel_t chan, mpc_chan_set *mpcchanset)
{
  int i, j;

  if (mpcchanset->max_index == -1)
    return;

  for (i = 0; i <= mpcchanset->max_index; i++)
    if (mpcchanset->chan_set[i].is_set == TRUE &&
	mpcchanset->chan_set[i].dest == dest &&
	mpcchanset->chan_set[i].channel == chan) {
      mpcchanset->chan_set[i].is_set = FALSE;

      if (mpcchanset->max_index == i) {
	for (j = i - 1; j >= 0; j--)
	  if (mpcchanset->chan_set[j].is_set == TRUE) {
	    mpcchanset->max_index = j;
	    return;
	  }

	mpcchanset->max_index = -1;
      }

      return;
    }
}

int
MPC_CHAN_ISSET(pnode_t dest, channel_t chan, mpc_chan_set *mpcchanset)
{
  int i;

  for (i = 0; i <= mpcchanset->max_index; i++)
    if (mpcchanset->chan_set[i].is_set == TRUE &&
	mpcchanset->chan_set[i].dest == dest &&
	mpcchanset->chan_set[i].channel == chan)
      return TRUE;
  return FALSE;
}

void
MPC_CHAN_ZERO(mpc_chan_set *mpcchanset)
{
  mpcchanset->max_index = -1;
}

int
mpc_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   mpc_chan_set *mpcreadfds, mpc_chan_set *mpcwritefds,
	   mpc_chan_set *mpcexceptfds, struct timeval *timeout)
{
  int ret;
  libmpc_select_t param;

  param.mpcchanset_in =  mpcreadfds;
  param.mpcchanset_ou =  mpcwritefds;
  param.mpcchanset_ex =  mpcexceptfds;
  if (mpcreadfds)   param.mpcchanset_in_max_index = mpcreadfds->max_index;
  if (mpcwritefds)  param.mpcchanset_ou_max_index = mpcwritefds->max_index;
  if (mpcexceptfds) param.mpcchanset_ex_max_index = mpcexceptfds->max_index;
  param.nd = nfds;
  param.in = readfds;
  param.ou = writefds;
  param.ex = exceptfds;
  param.tv = timeout;

  ret = ioctl(dev, LIBMPCSELECT, &param);

  if (ret < 0) return ret;
  else return param.retval;
}

#if 0
int
mpc_accept_channel(appclassname_t mainclass, appsubclassname_t subclass,
		   int blocking, uid_t *ret_uid,
		   pnode_t *ret_node, channel_t *ret_channel)
{
  mpcmsg_accept_t msg;

  msg.mainclass = mainclass;
  msg.subclass  = subclass;

  if (blocking) {
    command(MPCMSGACCEPTB, (caddr_t) &msg);
    if (msg.status == -1) {
      fprintf(mpc_stderr, "MPCLIB accept_channel");
      _exit(91);
    }
    return msg.pid;
  } else {
    command(MPCMSGACCEPTNB, (caddr_t) &msg);
    return msg.status;
  }
}
#endif


