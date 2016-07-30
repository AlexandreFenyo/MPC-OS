
/* $Id: socketwrap.c,v 1.2 1999/01/09 18:58:35 alex Exp $ */

#include <stdio.h>
#include <sys/errno.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sys/fcntl.h>

#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/filio.h>
#include <sys/sockio.h>

#include <stdarg.h>

#include <sys/time.h>
#include <netinet/in.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/ip_mroute.h>
#include <netinet/ip_mroute.h>
#include <arpa/inet.h>

#include <string.h>

#include "mpc.h"

/* #define WRAP_ONLY */

/* #define DEBUG_HSL */

extern char **environ;

static char environ_LD_PRELOAD[256];
static char environ_IP2PNODE[256];
static char environ_DONT_EXIT[32];
static char environ_DUMP_FILE[32];

#define IP2PNODE_SIZE 100 /* Should be equal or greater than MAX_NODES */
static struct in_addr ip2pnode[IP2PNODE_SIZE]; /* table indexed on node number */

/*
 * telnet/ftp can be used to test LD_PRELOAD.
 * # export DONT_EXIT=1
 * # export DUMP_FILE=1
 * # export MPCKEY=100
 * # export MPCSOCK=/tmp/mpc-100
 * # export LD_PRELOAD=./libsocketwrap.so.1.0 inetd -d ./inetd.conf
 */

static int dev = -1;

static FILE *mpc_stderr;

static u_short glob_cluster;
static pnode_t glob_pnode;
static int     glob_nclusters;

typedef struct _fd_tab_entry {
  int /*bool*/ in_use;
  int /*bool*/ connected;
  int /*bool*/ non_blocking;
  int /*bool*/ close_on_exec;
  channel_t channel;
  pnode_t node;
} fd_tab_entry_t;

static fd_tab_entry_t *fd_tab = NULL;

#define FD_TAB_START "FD_TAB="

static ssize_t (*p_write)(int, const void *, size_t)             = NULL;
static ssize_t (*p_read)(int, void *, size_t)                    = NULL;
static int     (*p_socket)(int, int, int)                        = NULL;
static int     (*p_close)(int)                                   = NULL;
static int     (*p_ioctl)(int, unsigned long, ...)               = NULL;
static int     (*p_listen)(int, int)                             = NULL;
static int     (*p_setsockopt)(int, int, int, const void *, int) = NULL;
static int     (*p_getsockopt)(int, int, int, void *, int *)     = NULL;
static int     (*p_accept)(int, struct sockaddr *, int *)        = NULL;
static int     (*p_bind)(int, const struct sockaddr *, int)      = NULL;
static int     (*p_connect)(int, const struct sockaddr *, int)   = NULL;
static int     (*p_getpeername)(int, struct sockaddr *, int *)   = NULL;
static int     (*p_getsockname)(int, struct sockaddr *, int *)   = NULL;
static ssize_t (*p_recv)(int, void *, size_t, int)               = NULL;
static ssize_t (*p_recvfrom)(int, void *, size_t, int,
			     struct sockaddr *, int *)           = NULL;
static ssize_t (*p_recvmsg)(int, struct msghdr *, int)           = NULL;
static ssize_t (*p_send)(int, const void *, size_t, int)         = NULL;
static ssize_t (*p_sendto)(int, const void *, size_t, int,
			   const struct sockaddr *, int)         = NULL;
static ssize_t (*p_sendmsg)(int, const struct msghdr *, int)     = NULL;
static int     (*p_execve)(const char *, char *const [],
			   char *const [])                       = NULL;
static pid_t   (*p_fork)(void)                                   = NULL;
static int     (*p_dup)(int)                                     = NULL;
static int     (*p_dup2)(int, int)                               = NULL;
static int     (*p_fcntl)(int, int, ...)                         = NULL;
static int     (*p_select)(int, fd_set *, fd_set *, fd_set *,
			   struct timeval *)                     = NULL;

static pnode_t
find_pnode(struct in_addr iaddr)
{
  int i;

  for (i = 0; i < IP2PNODE_SIZE; i++)
    if (!bcmp(ip2pnode + i, &iaddr, sizeof(struct in_addr)))
      return i;

  fprintf(mpc_stderr, "MPC-OS find_pnode() error (%s)\n",
	  inet_ntoa(iaddr));
  _exit(89);
}

static void
closedev(void)
{
#ifdef DEBUG_HSL
  fprintf(mpc_stderr, "MPC-OS exiting\n");
#endif

  mpc_close();

  if (dev >= 0) {
    close(dev);
    dev = -1;
  }
}

static void
opendev(void)
{
  int ret;
  int i;
  char ch[256];
  char *env;
  FILE *fip;
  char ips[256];
  int node;

  if (dev == -1) {
    dev = 1; /* Comment to test
       dev = open(HSL_DEVICE, O_RDWR);
       if (dev < 0) {
       dev = 1;*/ /* Avoid infinite loop */ /*
       _exit(98);
       }
    */

    putenv("LD_PRELOAD=");

    ret = atexit(closedev);
    if (ret) _exit(97);

    fd_tab = malloc(MAX_FD * sizeof(fd_tab_entry_t));
    if (fd_tab == NULL) _exit(95);
    bzero(fd_tab, MAX_FD * sizeof(fd_tab_entry_t));

    if (getenv("DUMP_FILE")) {
      sprintf(ch, "/tmp/sockmpc.%s", __progname);
      mpc_stderr = fopen(ch, "w");
      if (mpc_stderr == NULL) mpc_stderr = stderr;
    } else {
      mpc_stderr = stderr;
    }
    setvbuf(mpc_stderr, NULL, _IONBF, 0);

    env = getenv("LD_PRELOAD");
    if (env && strlen(env) < sizeof(environ_LD_PRELOAD) - 16)
      sprintf(environ_LD_PRELOAD, "LD_PRELOAD=%s", env);

    env = getenv("IP2PNODE");
    if (env && strlen(env) < sizeof(environ_IP2PNODE) - 16)
      sprintf(environ_IP2PNODE, "IP2PNODE=%s", env);

    env = getenv("DONT_EXIT");
    if (env && strlen(env) < sizeof(environ_DONT_EXIT) - 16)
      sprintf(environ_DONT_EXIT, "DONT_EXIT=%s", env);

    env = getenv("DUMP_FILE");
    if (env && strlen(env) < sizeof(environ_DUMP_FILE) - 16)
      sprintf(environ_DUMP_FILE, "DUMP_FILE=%s", env);

    p_write = dlsym(RTLD_NEXT, "_write");
    /* Don't print anything to avoid infinite loop... */
    if (!p_write) _exit(99);

    p_read = dlsym(RTLD_NEXT, "_read");
    if (!p_read) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_socket = dlsym(RTLD_NEXT, "_socket");
    if (!p_socket) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_close = dlsym(RTLD_NEXT, "_close");
    if (!p_close) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_ioctl = dlsym(RTLD_NEXT, "_ioctl");
    if (!p_ioctl) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_listen = dlsym(RTLD_NEXT, "_listen");
    if (!p_listen) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_setsockopt = dlsym(RTLD_NEXT, "_setsockopt");
    if (!p_setsockopt) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_getsockopt = dlsym(RTLD_NEXT, "_getsockopt");
    if (!p_getsockopt) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_accept = dlsym(RTLD_NEXT, "_accept");
    if (!p_accept) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_bind = dlsym(RTLD_NEXT, "_bind");
    if (!p_bind) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_connect = dlsym(RTLD_NEXT, "_connect");
    if (!p_connect) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_getpeername = dlsym(RTLD_NEXT, "_getpeername");
    if (!p_getpeername) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_getsockname = dlsym(RTLD_NEXT, "_getsockname");
    if (!p_getsockname) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_recv = dlsym(RTLD_NEXT, "_recv");
    if (!p_recv) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_recvfrom = dlsym(RTLD_NEXT, "_recvfrom");
    if (!p_recvfrom) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_recvmsg = dlsym(RTLD_NEXT, "_recvmsg");
    if (!p_recvmsg) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_send = dlsym(RTLD_NEXT, "_send");
    if (!p_send) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_sendto = dlsym(RTLD_NEXT, "_sendto");
    if (!p_sendto) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_sendmsg = dlsym(RTLD_NEXT, "_sendmsg");
    if (!p_sendmsg) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_execve = dlsym(RTLD_NEXT, "_execve");
    if (!p_execve) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_fork = dlsym(RTLD_NEXT, "_fork");
    if (!p_fork) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_dup = dlsym(RTLD_NEXT, "_dup");
    if (!p_dup) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_dup2 = dlsym(RTLD_NEXT, "_dup2");
    if (!p_dup2) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_fcntl = dlsym(RTLD_NEXT, "_fcntl");
    if (!p_fcntl) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

    p_select = dlsym(RTLD_NEXT, "_select");
    if (!p_select) {
      fputs(dlerror(), mpc_stderr);
      fputs("\n",      mpc_stderr);
      _exit(96);
    }

#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS catched %s\n", __progname);
#endif

    if (getenv("FD_TAB")) {
      char *src = getenv("FD_TAB");
      char *dst = (char *) fd_tab;

      fprintf(mpc_stderr, "FD_TAB present\n");

      for (i = 0; i < MAX_FD * sizeof(fd_tab_entry_t); i++)
	dst[i] = ((src[2 * i] - 'A')<<4) | (src[2 * i + 1] - 'A');

      for (i = 0; i < MAX_FD; i++)
	if (fd_tab[i].in_use)
	  fprintf(mpc_stderr, "reference from parent (fd=%d)\n", i);
    }

    bzero(ip2pnode, sizeof(ip2pnode));
    fip = fopen(getenv("IP2PNODE") ? getenv("IP2PNODE") : "/tmp/ip2pnode.conf",
	       "r");
    if (fip == NULL) {
      MPC_PERROR("open IP2PNODE");
      _exit(89);
    }

    while (EOF != fscanf(fip, "%s %d\n", ips, &node)) {
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS scanning IP2PNODE(%s -> %d)\n", ips, node);
#endif
      if (node >= IP2PNODE_SIZE) {
	fprintf(mpc_stderr, "MPC-OS node too big\n");
	_exit(89);
      }

      ret = inet_aton(ips, ip2pnode + node);
      if (ret == INADDR_NONE) {
	fprintf(mpc_stderr, "MPC-OS inet_aton error\n");
	_exit(89);
      }
    }
    
    fclose(fip);

    mpc_init();
    mpc_get_local_infos(&glob_cluster, &glob_pnode, &glob_nclusters);
  }
}

/*
 * ___CTOR_LIST__ is not honored if a library containing this object
 * is loaded with LD_PRELOAD :
 * we need to call opendev() at the begining of each overriden function...
 */

asm(".stabs \"___CTOR_LIST__\",23,0,0,_opendev");

/************************************************************/

ssize_t
read(int d, void *buf, size_t nbytes)
{
  int ret;

  opendev();

  if (d >= 0 && d < MAX_FD && fd_tab[d].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS read(%d, %p, %d)\n", d, buf, nbytes);
#endif

#ifdef WRAP_ONLY
    ret = p_read(d, buf, nbytes);
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS read -> %d\n", ret);
#endif
    return ret;
#else
    if (fd_tab[d].connected == TRUE) {
      ret = mpc_read(fd_tab[d].node, fd_tab[d].channel, buf, nbytes);
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS mpc_read -> %d\n", ret);
#endif
      return ret;
    }
    else {
      ret = p_read(d, buf, nbytes);
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS read -> %d\n", ret);
#endif
      return ret;
    }
#endif
  }

  return p_read(d, buf, nbytes);
}

ssize_t
write(int d, const void *buf, size_t nbytes)
{
  int ret;

  opendev();

  if (d >= 0 && d < MAX_FD && fd_tab[d].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS write(%d, %p, %d)\n", d, buf, nbytes);
#endif

#ifdef WRAP_ONLY
    ret = p_write(d, buf, nbytes);
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS write -> %d\n", ret);
#endif
    return ret;
#else
    if (fd_tab[d].connected == TRUE) {
      ret = mpc_write(fd_tab[d].node, fd_tab[d].channel, buf, nbytes);
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS mpc_write -> %d\n", ret);
#endif
      return ret;
    }
    else {
      ret = p_write(d, buf, nbytes);
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS write -> %d\n", ret);
#endif
      return ret;
    }
#endif
  }

  return p_write(d, buf, nbytes);
}

int
socket(int domain, int type, int protocol)
{
  int ret;

  opendev();

  ret = p_socket(domain, type, protocol);
  if (ret >= 0 && ret < MAX_FD && domain == PF_INET && type == SOCK_STREAM) {
    bzero(fd_tab + ret, sizeof(fd_tab_entry_t));
    fd_tab[ret].in_use = TRUE;
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS socket -> %d\n", ret);
#endif
  }

  return ret;
}

int
close(int d)
{
  opendev();

  if (d >= 0 && d < MAX_FD && fd_tab[d].in_use == TRUE) {
    bzero(fd_tab + d, sizeof(fd_tab_entry_t));
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS close(%d)\n", d);
#endif

    return p_close(d);
  }

  return p_close(d);
}

/* See sys/filio.h, sys/sockio.h for ioctl's definition */

int
ioctl(int d, unsigned long request, ...)
{
  char *argp;
  va_list ap;

  opendev();

  va_start(ap, request);
  argp = va_arg(ap, char *);
  va_end(ap);

  if (d >= 0 && d < MAX_FD && fd_tab[d].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS ioctl(%d, %ld, %p)\n", d, request, argp);

    switch(request) {
    case FIOCLEX:
      fprintf(mpc_stderr, "MPC-OS ioctl FIOCLEX\n");
      break;

    case FIONCLEX:
      fprintf(mpc_stderr, "MPC-OS ioctl FIONCLEX\n");
      break;

    case FIONREAD:
      fprintf(mpc_stderr, "MPC-OS ioctl FIONREAD\n");
      break;

    case FIONBIO:
      fprintf(mpc_stderr, "MPC-OS ioctl FIONBIO (%d)\n", *(int *) argp);
      break;

    case FIOASYNC:
      fprintf(mpc_stderr, "MPC-OS ioctl FIOASYNC\n");
      break;

    case FIOSETOWN:
      fprintf(mpc_stderr, "MPC-OS ioctl FIOSETOWN\n");
      break;

    case FIOGETOWN:
      fprintf(mpc_stderr, "MPC-OS ioctl FIOGETOWN\n");
      break;

    case SIOCSHIWAT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSHIWAT\n");
      break;

    case SIOCGHIWAT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGHIWAT\n");
      break;

    case SIOCSLOWAT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSLOWAT\n");
      break;

    case SIOCGLOWAT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGLOWAT\n");
      break;

    case SIOCATMARK:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCATMARK\n");
      break;

    case SIOCSPGRP:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSPGRP\n");
      break;

    case SIOCGPGRP:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGPGRP\n");
      break;

    case SIOCADDRT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCADDRT\n");
      break;

    case SIOCDELRT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCDELRT\n");
      break;

    case SIOCGETVIFCNT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGETVIFCNT\n");
      break;

    case SIOCGETSGCNT:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGETSGCNT\n");
      break;

    case SIOCSIFADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFADDR\n");
      break;

    case OSIOCGIFADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl OSIOCGIFADDR\n");
      break;

    case SIOCGIFADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFADDR\n");
      break;

    case SIOCSIFDSTADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFDSTADDR\n");
      break;

    case OSIOCGIFDSTADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl OSIOCGIFDSTADDR\n");
      break;

    case SIOCGIFDSTADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFDSTADDR\n");
      break;

    case SIOCSIFFLAGS:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFFLAGS\n");
      break;

    case SIOCGIFFLAGS:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFFLAGS\n");
      break;

    case OSIOCGIFBRDADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl OSIOCGIFBRDADDR\n");
      break;

    case SIOCGIFBRDADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFBRDADDR\n");
      break;

    case SIOCSIFBRDADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFBRDADDR\n");
      break;

    case OSIOCGIFCONF:
      fprintf(mpc_stderr, "MPC-OS ioctl OSIOCGIFCONF\n");
      break;

    case SIOCGIFCONF:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFCONF\n");
      break;

    case OSIOCGIFNETMASK:
      fprintf(mpc_stderr, "MPC-OS ioctl OSIOCGIFNETMASK\n");
      break;

    case SIOCGIFNETMASK:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFNETMASK\n");
      break;

    case SIOCSIFNETMASK:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFNETMASK\n");
      break;

    case SIOCGIFMETRIC:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFMETRIC\n");
      break;

    case SIOCSIFMETRIC:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFMETRIC\n");
      break;

    case SIOCDIFADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCDIFADDR\n");
      break;

    case SIOCAIFADDR:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCAIFADDR\n");
      break;

    case SIOCADDMULTI:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCADDMULTI\n");
      break;

    case SIOCDELMULTI:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCDELMULTI\n");
      break;

    case SIOCGIFMTU:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFMTU\n");
      break;

    case SIOCSIFMTU:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFMTU\n");
      break;

    case SIOCGIFPHYS:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFPHYS\n");
      break;

    case SIOCSIFPHYS:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFPHYS\n");
      break;

    case SIOCSIFMEDIA:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCSIFMEDIA\n");
      break;

    case SIOCGIFMEDIA:
      fprintf(mpc_stderr, "MPC-OS ioctl SIOCGIFMEDIA\n");
      break;

    default:
      fprintf(mpc_stderr, "MPC-OS unknown ioctl\n");
      break;
    }
#endif

    if (request == FIONBIO)
      fd_tab[d].non_blocking = (*(int *) argp) ? TRUE : FALSE;

    return p_ioctl(d, request, argp);
  }

  return p_ioctl(d, request, argp);
}

int
listen(int s, int backlog)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS listen(%d, %d)\n", s, backlog);
#endif

    return p_listen(s, backlog);
  }

  return p_listen(s, backlog);
}

/* See sys/socket.h for option names values */

#ifdef DEBUG_HSL
static void socklog(int level, int optname)
{
  switch (level) {
  case SOL_SOCKET:
    fprintf(mpc_stderr, "MPC-OS level SOL_SOCKET\n");
    break;

  case IPPROTO_IP:
    fprintf(mpc_stderr, "MPC-OS level IPPROTO_IP\n");
    break;
  }

  if (level == IPPROTO_IP)
    switch (optname) {
    case IP_TOS:
      fprintf(mpc_stderr, "MPC-OS optname IP_TOS\n");
      break;

    default:
      fprintf(mpc_stderr, "MPC-OS unknown optname\n");
      break;
    }

  if (level != IPPROTO_IP)
    switch (optname) {
    case SO_DEBUG:
      fprintf(mpc_stderr, "MPC-OS optname SO_DEBUG\n");
      break;

    case SO_REUSEADDR:
      fprintf(mpc_stderr, "MPC-OS optname SO_REUSEADDR\n");
      break;

    case SO_REUSEPORT:
      fprintf(mpc_stderr, "MPC-OS optname SO_REUSEPORT\n");
      break;

    case SO_KEEPALIVE:
      fprintf(mpc_stderr, "MPC-OS optname SO_KEEPALIVE\n");
      break;

    case SO_DONTROUTE:
      fprintf(mpc_stderr, "MPC-OS optname SO_DONTROUTE\n");
      break;

    case SO_LINGER:
      fprintf(mpc_stderr, "MPC-OS optname SO_LINGER\n");
      break;

    case SO_BROADCAST:
      fprintf(mpc_stderr, "MPC-OS optname SO_BROADCAST\n");
      break;

    case SO_OOBINLINE:
      fprintf(mpc_stderr, "MPC-OS optname SO_OOBINLINE\n");
      break;

    case SO_SNDBUF:
      fprintf(mpc_stderr, "MPC-OS optname SO_SNDBUF\n");
      break;

    case SO_RCVBUF:
      fprintf(mpc_stderr, "MPC-OS optname SO_RCVBUF\n");
      break;

    case SO_SNDLOWAT:
      fprintf(mpc_stderr, "MPC-OS optname SO_SNDLOWAT\n");
      break;

    case SO_RCVLOWAT:
      fprintf(mpc_stderr, "MPC-OS optname SO_RCVLOWAT\n");
      break;

    case SO_SNDTIMEO:
      fprintf(mpc_stderr, "MPC-OS optname SO_SNDTIMEO\n");
      break;

    case SO_RCVTIMEO:
      fprintf(mpc_stderr, "MPC-OS optname SO_RCVTIMEO\n");
      break;

    case SO_TYPE:
      fprintf(mpc_stderr, "MPC-OS optname SO_TYPE\n");
      break;

    case SO_ERROR:
      fprintf(mpc_stderr, "MPC-OS optname SO_ERROR\n");
      break;

    default:
      fprintf(mpc_stderr, "MPC-OS unknown optname\n");
      break;
    }
}
#endif

int
setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS setsockopt(%d, %d, %d, %p, %d)\n",
	    s, level, optname, optval, optlen);
    socklog(level, optname);
#endif

    return p_setsockopt(s, level, optname, optval, optlen);
  }

  return p_setsockopt(s, level, optname, optval, optlen);
}

int
getsockopt(int s, int level, int optname, void *optval, int *optlen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS getsockopt(%d, %d, %d, %p, %p)\n",
	    s, level, optname, optval, optlen);
    socklog(level, optname);
#endif

    return p_getsockopt(s, level, optname, optval, optlen);
  }

  return p_getsockopt(s, level, optname, optval, optlen);
}

int
fcntl(int fd, int cmd, ...)
{
  int ret;
  int arg;
  va_list ap;

  opendev();

  va_start(ap, cmd);
  arg = va_arg(ap, int);
  va_end(ap);

  if (fd >= 0 && fd < MAX_FD && fd_tab[fd].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS fcntl(%d, %d, %d)\n",
	    fd, cmd, arg);

    switch (cmd) {
    case F_DUPFD:
      fprintf(mpc_stderr, "MPC-OS command F_DUPFD\n");
      break;

    case F_GETFD:
      fprintf(mpc_stderr, "MPC-OS command F_GETFD\n");
      break;

    case F_SETFD:
      fprintf(mpc_stderr, "MPC-OS command F_SETFD\n");
      break;

    case F_GETFL:
      fprintf(mpc_stderr, "MPC-OS command F_GETFL\n");
      break;

    case F_SETFL:
      fprintf(mpc_stderr, "MPC-OS command F_SETFL\n");
      break;

    case F_GETOWN:
      fprintf(mpc_stderr, "MPC-OS command F_GETOWN\n");
      break;

    case F_SETOWN:
      fprintf(mpc_stderr, "MPC-OS command F_SETOWN\n");
      break;

    case F_GETLK:
      fprintf(mpc_stderr, "MPC-OS command F_GETLK\n");
      break;

    case F_SETLK:
      fprintf(mpc_stderr, "MPC-OS command F_SETLK\n");
      break;

    case F_SETLKW:
      fprintf(mpc_stderr, "MPC-OS command F_DUPFD\n");
      break;

    default:
      fprintf(mpc_stderr, "MPC-OS unknown command\n");
      break;
    }
#endif

    ret = p_fcntl(fd, cmd, arg);
    return ret;
  }

  return p_fcntl(fd, cmd, arg);
}

int
accept(int s, struct sockaddr *addr, int *addrlen)
{
  int newsock, ret;
  appsubclassname_t subclassname;
  struct sockaddr_in sa;
  int namelen;
  pnode_t dist_pnode;
  pnode_t local_pnode;
  u_short dist_port;
  u_short local_port;
  primary_t primary;
  channel_t chan[2];

  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS accept(%d, %p, %p)\n",
	    s, addr, addrlen);
#endif

    newsock = p_accept(s, addr, addrlen);
    if (newsock < 0) return newsock;

#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS accept -> %d\n", newsock);
#endif

    bzero(fd_tab + newsock, sizeof(fd_tab_entry_t));
    fd_tab[newsock].in_use = TRUE;

    namelen = sizeof(sa);
    ret = p_getpeername(newsock, (struct sockaddr *) &sa, &namelen);
    if (ret < 0) {
      MPC_PERROR("p_getpeername");
      _exit(89);
    }
    dist_pnode = find_pnode(sa.sin_addr);
    dist_port = sa.sin_port;

    namelen = sizeof(sa);
    ret = p_getsockname(newsock, (struct sockaddr *) &sa, &namelen);
    if (ret < 0) {
      MPC_PERROR("p_getsockname");
      _exit(89);
    }
    local_pnode = find_pnode(sa.sin_addr);
    local_port = sa.sin_port;

    if (dist_pnode < local_pnode) {
      ((int *) &primary)[0] = dist_pnode | (dist_port<<16);
      ((int *) &primary)[1] = local_pnode | (local_port<<16);
    } else {
      ((int *) &primary)[0] = local_pnode | (local_port<<16);
      ((int *) &primary)[1] = dist_pnode | (dist_port<<16);
    }

#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS PRIMARY accept (%#x:%#x/%#x:%#x)\n",
	    local_pnode, local_port, dist_pnode, dist_port);
    fprintf(mpc_stderr, "MPC-OS PRIMARY accept {%#x/%#x}\n",
	    (int) (primary & 0xFFFFFFFF), (int) (primary>>32));
#endif

    subclassname = make_subclass_prefnode(glob_cluster, dist_pnode, primary);
    ret = mpc_get_channel(APPCLASS_INTERNET, subclassname,
			  &chan[0], &chan[1], HSL_PROTO_MDCP);
    if (ret) {
      fprintf(mpc_stderr, "MPC-OS mpc_get_channel error\n");
      _exit(89);
    }

    fd_tab[newsock].connected = TRUE;
    fd_tab[newsock].channel = chan[0];
    fd_tab[newsock].node = dist_pnode;

    return newsock;
  }

  return p_accept(s, addr, addrlen);
}

int
bind(int s, const struct sockaddr *name, int namelen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS bind(%d, %p, %d)\n",
	    s, name, namelen);
#endif

    return p_bind(s, name, namelen);
  }

  return p_bind(s, name, namelen);
}

int
connect(int s, const struct sockaddr *name, int namelen)
{
  int ret;
  appsubclassname_t subclassname;
  struct sockaddr_in sa;
  int len;
  pnode_t dist_pnode;
  pnode_t local_pnode;
  u_short dist_port;
  u_short local_port;
  primary_t primary;
  channel_t chan[2];

  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS connect(%d, %p, %d)\n",
	    s, name, namelen);
#endif

    if (fd_tab[s].in_use == FALSE) {
      fprintf(mpc_stderr, "MPC-OS connect error\n");
      _exit(89);
    }

    ret = p_connect(s, name, namelen);
    if (ret < 0) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS connect -> %d\n", ret);
    MPC_PERROR("connect");
#endif
      return ret;
    }

    len = sizeof(sa);
    ret = p_getpeername(s, (struct sockaddr *) &sa, &len);
    if (ret < 0) {
      MPC_PERROR("p_getpeername");
      _exit(89);
    }
    dist_pnode = find_pnode(sa.sin_addr);
    dist_port = sa.sin_port;

    len = sizeof(sa);
    ret = p_getsockname(s, (struct sockaddr *) &sa, &len);
    if (ret < 0) {
      MPC_PERROR("p_getsockname");
      _exit(89);
    }
    local_pnode = find_pnode(sa.sin_addr);
    local_port = sa.sin_port;

    if (dist_pnode < local_pnode) {
      ((int *) &primary)[0] = dist_pnode | (dist_port<<16);
      ((int *) &primary)[1] = local_pnode | (local_port<<16);
    } else {
      ((int *) &primary)[0] = local_pnode | (local_port<<16);
      ((int *) &primary)[1] = dist_pnode | (dist_port<<16);
    }

#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS PRIMARY connect (%#x:%#x/%#x:%#x)\n",
	    local_pnode, local_port, dist_pnode, dist_port);
    fprintf(mpc_stderr, "MPC-OS PRIMARY connect {%#x/%#x})\n",
	    (int) (primary & 0xFFFFFFFF), (int) (primary>>32));
#endif

    subclassname = make_subclass_prefnode(glob_cluster, dist_pnode, primary);
    ret = mpc_get_channel(APPCLASS_INTERNET, subclassname,
			  &chan[0], &chan[1], HSL_PROTO_MDCP);
    if (ret) {
      fprintf(mpc_stderr, "MPC-OS mpc_get_channel error\n");
      _exit(89);
    }

    fd_tab[s].connected = TRUE;
    fd_tab[s].channel = chan[0];
    fd_tab[s].node = dist_pnode;

    return 0;
  }

  return p_connect(s, name, namelen);
}

int
getpeername(int s, struct sockaddr *name, int *namelen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS getpeername(%d, %p, %p)\n",
	    s, name, namelen);
#endif

    return p_getpeername(s, name, namelen);
  }

  return p_getpeername(s, name, namelen);
}

int
getsockname(int s, struct sockaddr *name, int *namelen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS getsockname(%d, %p, %p)\n",
	    s, name, namelen);
#endif

    return p_getsockname(s, name, namelen);
  }

  return p_getsockname(s, name, namelen);
}

ssize_t
recv(int s, void *buf, size_t len, int flags)
{
  int ret;

  opendev();

  /* WARNING: ONLY WORKS WITH the ch_p4 driver of MPICH !!!! */
  if (flags == MSG_PEEK) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS MSGPEEK -> 1\n");
#endif
    return 1;
  }

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS recv(%d, %p, %d, %d)\n",
	    s, buf, len, flags);
#endif

    ret = p_recv(s, buf, len, flags);
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS recv -> %d\n", ret);
#endif
    return ret;
  }

  ret = p_recv(s, buf, len, flags);
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS recv -> %d\n", ret);
#endif
  return ret;
}

ssize_t
recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from,
	 int *fromlen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS recvfrom(%d, %p, %d, %d, %p, %p)\n",
	    s, buf, len, flags, from, fromlen);
#endif

    return p_recvfrom(s, buf, len, flags, from, fromlen);
  }

  return p_recvfrom(s, buf, len, flags, from, fromlen);
}

ssize_t
recvmsg(int s, struct msghdr *msg, int flags)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS recvmsg(%d, %p, %d)\n",
	    s, msg, flags);
#endif

    return p_recvmsg(s, msg, flags);
  }

  return p_recvmsg(s, msg, flags);
}

ssize_t
send(int s, const void *msg, size_t len, int flags)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS send(%d, %p, %d, %d)\n",
	    s, msg, len, flags);
#endif

    return p_send(s, msg, len, flags);
  }

  return p_send(s, msg, len, flags);
}

ssize_t
sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to,
	 int tolen)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS sendto(%d, %p, %d, %d, %p, %p)\n",
	    s, msg, len, flags, to, tolen);
#endif

    return p_sendto(s, msg, len, flags, to, tolen);
  }

  return p_sendto(s, msg, len, flags, to, tolen);
}

ssize_t
sendmsg(int s, const struct msghdr *msg, int flags)
{
  opendev();

  if (s >= 0 && s < MAX_FD && fd_tab[s].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS sendmsg(%d, %p, %d)\n",
	    s, msg, flags);
#endif

    return p_sendmsg(s, msg, flags);
  }

  return p_sendmsg(s, msg, flags);
}

int
dup(int oldd)
{
  opendev();

  if (oldd >= 0 && oldd < MAX_FD && fd_tab[oldd].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS dup(%d)\n", oldd);
#endif

    return p_dup(oldd);
  }

  return p_dup(oldd);
}

int
dup2(int oldd, int newd)
{
  opendev();

  if (oldd >= 0 && oldd < MAX_FD && fd_tab[oldd].in_use == TRUE) {
#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS dup2(%d, %d)\n", oldd, newd);
#endif

    return p_dup2(oldd, newd);
  }

  return p_dup2(oldd, newd);
}

int
select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
  int i, ret;
  mpc_chan_set readchs, writechs, exceptchs;
  fd_set copy_readfds, copy_writefds, copy_exceptfds;

  opendev();

#ifdef DEBUG_HSL
    fprintf(mpc_stderr, "MPC-OS select() nfds=%d\n", nfds);
#endif

#ifdef WRAP_ONLY
  ret = p_select(nfds, readfds, writefds, exceptfds, timeout);

#ifdef DEBUG_HSL
  for (i = 0; i < nfds; i++) {
    if (readfds != NULL && FD_ISSET(i, readfds))
      fprintf(mpc_stderr, "MPC-OS select read : fd=%d\n", i);
    if (writefds != NULL && FD_ISSET(i, writefds))
      fprintf(mpc_stderr, "MPC-OS select write : fd=%d\n", i);
    if (exceptfds != NULL && FD_ISSET(i, exceptfds))
      fprintf(mpc_stderr, "MPC-OS select except : fd=%d\n", i);
    }
  fprintf(mpc_stderr, "MPC-OS select -> %d\n", ret);
#endif

  return ret;
#else

  if (readfds != NULL) copy_readfds = *readfds;
  if (writefds != NULL) copy_writefds = *writefds;
  if (exceptfds != NULL) copy_exceptfds = *exceptfds;

  MPC_CHAN_ZERO(&readchs);
  MPC_CHAN_ZERO(&writechs);
  MPC_CHAN_ZERO(&exceptchs);

#ifdef DEBUG_HSL
  for (i = 0; i < nfds; i++) {
    if (readfds != NULL && FD_ISSET(i, readfds))
      fprintf(mpc_stderr, "MPC-OS select read : fd=%d\n", i);
    if (writefds != NULL && FD_ISSET(i, writefds))
      fprintf(mpc_stderr, "MPC-OS select write : fd=%d\n", i);
    if (exceptfds != NULL && FD_ISSET(i, exceptfds))
      fprintf(mpc_stderr, "MPC-OS select except : fd=%d\n", i);
    }
#endif

  for (i = 0; i < nfds; i++) {
    if (readfds != NULL && FD_ISSET(i, readfds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE) {
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS select read connected : fd=%d\n", i);
#endif

      MPC_CHAN_SET(fd_tab[i].node, fd_tab[i].channel, &readchs);
      FD_CLR(i, readfds);
    }

    if (writefds != NULL && FD_ISSET(i, writefds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE) {
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS select write connected : fd=%d\n", i);
#endif

      MPC_CHAN_SET(fd_tab[i].node, fd_tab[i].channel, &writechs);
      FD_CLR(i, writefds);
    }

    if (exceptfds != NULL && FD_ISSET(i, exceptfds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE) {
#ifdef DEBUG_HSL
      fprintf(mpc_stderr, "MPC-OS select except connected : fd=%d\n", i);
#endif

      MPC_CHAN_SET(fd_tab[i].node, fd_tab[i].channel, &exceptchs);
      FD_CLR(i, exceptfds);
    }
  }

  ret = mpc_select(nfds, readfds, writefds, exceptfds,
		   (readchs.max_index < 0) ? NULL : &readchs,
		   (writechs.max_index < 0) ? NULL : &writechs,
		   (exceptchs.max_index < 0) ? NULL : &exceptchs,
		   timeout);
  if (ret < 0) {
#ifdef DEBUG_HSL
    MPC_PERROR("mpc_select");
#endif

    return ret;
  }

#ifdef DEBUG_HSL
  fprintf(mpc_stderr, "MPC-OS mpc_select -> %d\n", ret);
#endif

  for (i = 0; i < nfds; i++) {
    if (readfds != NULL && FD_ISSET(i, &copy_readfds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE &&
	MPC_CHAN_ISSET(fd_tab[i].node, fd_tab[i].channel, &readchs))
      FD_SET(i, readfds);

    if (writefds != NULL && FD_ISSET(i, &copy_writefds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE &&
	MPC_CHAN_ISSET(fd_tab[i].node, fd_tab[i].channel, &writechs))
      FD_SET(i, writefds);

    if (exceptfds != NULL && FD_ISSET(i, &copy_exceptfds) &&
	fd_tab[i].in_use == TRUE && fd_tab[i].connected == TRUE &&
	MPC_CHAN_ISSET(fd_tab[i].node, fd_tab[i].channel, &exceptchs))
      FD_SET(i, exceptfds);
  }

  return ret;

#endif
}

/************************************************************/

#if 0

ssize_t
execve(const char *path, char *const argv[], char *const envp[])
{
  int i, j;
  char c;
  char *ch;
  char *ch_cursor;

  static char *newenv[1024];
  static char newenv_str[65536];
  char *newenv_str_p = newenv_str;

  opendev();

#if 0
  if (getenv("DUMP_FILE")) {
    sprintf(ch, "/tmp/sockmpc.%s", __progname);
    mpc_stderr = fopen(ch, "w+");
    if (mpc_stderr == NULL) mpc_stderr = stderr;
  } else {
    mpc_stderr = stderr;
  }
  setvbuf(mpc_stderr, NULL, _IONBF, 0);
#endif

#ifdef DEBUG_HSL
  fprintf(mpc_stderr, "MPC-OS@%s execve(%s, %p, %p)\n",
	  __progname, path, argv, envp);
#endif

  if (envp != environ) fprintf(mpc_stderr, "MPC-OS envp != environ\n");
  if (!getenv("LD_PRELOAD")) {
    fprintf(mpc_stderr,
	    "MPC-OS LD_PRELOAD not in environment\n");
  }


  for (i = 0; i < 65536; i++) {
    if (envp[i] == NULL) break;

    newenv[i] = newenv_str_p;
    if (strlen(envp[i]) >= sizeof(newenv_str) - (newenv[i] - newenv_str) - 1) {
      fprintf(mpc_stderr,
	      "MPC-OS new environment not big enough\n");
      break;
    }
    strncpy(newenv[i], envp[i],
	    sizeof(newenv_str) - (newenv[i] - newenv_str) - 1);

    newenv_str_p += strlen(newenv[i]) + 1;
  }

  if (i >= 65500)
    fprintf(mpc_stderr, "MPC-OS too many environment variables\n");
  else {
    newenv[i++] = environ_LD_PRELOAD;
    newenv[i++] = environ_DONT_EXIT;
    newenv[i++] = environ_DUMP_FILE;
  }
  newenv[i] = NULL;

  ch = malloc(MAX_FD * sizeof(fd_tab_entry_t) * 2 + strlen(FD_TAB_START) + 1);

  if (ch) {
    strncpy(ch, FD_TAB_START, strlen(FD_TAB_START));
    ch_cursor = ch + strlen(FD_TAB_START);
    for (j = 0; j < MAX_FD * sizeof(fd_tab_entry_t); j++) {
      c = ((char *) fd_tab)[j];
      *(ch_cursor++) = (c>>4) + 'A';
      *(ch_cursor++) = (c & 0x0f) + 'A';
    }
    *(ch_cursor++) = '\0';

    newenv[i++] = ch;
    newenv[i] = NULL;
  } else
    fprintf(mpc_stderr, "MPC-OS can't malloc\n");

  free(ch);

#ifdef DEBUG_HSL
 fprintf(mpc_stderr, environ_LD_PRELOAD);
 fprintf(mpc_stderr, "\n");
 fprintf(mpc_stderr, environ_DONT_EXIT);
 fprintf(mpc_stderr, "\n");
 fprintf(mpc_stderr, environ_DUMP_FILE);
 fprintf(mpc_stderr, "\n");
 {
   char *const *ch;
   int i;
   ch = argv;
   while (ch && *ch) {
     fprintf(mpc_stderr, "arg>>%s\n", *ch);
     ch++; 
   }
   ch = envp;
   while (ch && *ch) {
     fprintf(mpc_stderr, "env>>%s\n", *ch);
     ch++; 
   }
   /*
   ch = newenv;
   while (ch && *ch) {
     fprintf(mpc_stderr, "new>>%s\n", *ch);
     ch++; 
   }
   */
 }
#endif

  return p_execve(path, argv, newenv);
}

#endif

pid_t
fork(void)
{
  fd_tab_entry_t *old_fd_tab;
  pid_t ret;

  opendev();

#ifdef DEBUG_HSL
  fprintf(mpc_stderr, "MPC-OS fork()\n");
#endif

  ret = p_fork();

  return ret;

#if 0

  if (!ret) {
    /* Child */

    old_fd_tab = fd_tab;
    fd_tab = malloc(MAX_FD * sizeof(fd_tab_entry_t));
    if (fd_tab == NULL) _exit(94);

    if (old_fd_tab)
      memcpy(fd_tab, old_fd_tab, MAX_FD * sizeof(fd_tab_entry_t));
    else
      bzero(fd_tab, MAX_FD * sizeof(fd_tab_entry_t));

    return ret;
  } else {
    /* Parent */
    return ret;
  }

#endif

}


