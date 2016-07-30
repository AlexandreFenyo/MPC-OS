
/* $Id: driver.c,v 1.8 1999/09/19 16:16:35 alex Exp $ */

/*******************************************************************
 * AUTHOR : Alexandre Fenyö (Alexandre.Fenyo@lip6.fr)              *
 *******************************************************************/

#include <sys/cdefs.h>
#include <sys/types.h>

#include <sys/sysent.h>
#include <sys/param.h>
#ifndef _SMP_SUPPORT
#include <sys/ioctl.h>
#endif
#include <sys/systm.h>

#define ACTUALLY_LKM_NOT_KERNEL
#include <sys/signalvar.h>
#include <sys/conf.h>

#include <sys/mount.h>
#include <sys/exec.h>
#include <sys/lkm.h>
#include <a.out.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <vm/vm.h>

#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <sys/lock.h>
#endif
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <machine/pmap.h>
#include <sys/errno.h>

#include <vm/vm_prot.h>
#include <vm/vm_object.h>

#include <sys/kernel.h>

#ifdef _SMP_SUPPORT
#include <sys/uio.h>
#endif

#include "/sys/i386/isa/isa_device.h"
#include "/sys/pci/pcivar.h"
#include "/sys/pci/pcireg.h"
#ifndef _SMP_SUPPORT
#include "/usr/src/sys/i386/isa/pcibus.h"
#endif

#include "../MPC_OS/mpcshare.h"

#include "data.h"
#include "driver.h"
#include "put.h"
#include "ddslrpp.h"
#include "ddslrpv.h"
#include "mdcp.h"

char debug_stage [DEBUG_STAGE_SIZE];
char debug_stage2[DEBUG_STAGE_SIZE];

static caddr_t protocol_names[] = { "PUT",
				    "SLRP/P", "SCP/P",
				    "SLRP/V", "SCP/V",
				    "MDCP" };

static int open_count = 0;
static int cpt = 0;
static int cpt2 = 0;

int perfmon_invalid = 0;
struct timeval perfmon_tv_start;
struct timeval perfmon_tv_end;

#ifndef _SMP_SUPPORT
static struct linker_set *pcibus_set_p;
#endif

static int dev_open();
static int dev_close();
static int dev_read();
static int dev_ioctl();

#ifdef _SMP_SUPPORT
static int
noselect(dev, rw, p)
        dev_t dev;
        int rw;
        struct proc *p;
{
        printf("noselect(0x%x) called\n", dev);
        /* XXX is this distinguished from 1 for data available? */
        return (ENXIO);
}
#endif

static struct cdevsw cdev = {
  dev_open,
  dev_close,
  dev_read,
#ifndef _SMP_SUPPORT
  nxwrite,
#else
  nowrite,
#endif
  dev_ioctl,
#ifndef _SMP_SUPPORT
  nxstop,
#else
  nostop,
#endif
#ifndef _SMP_SUPPORT
  nxreset,
#else
  noreset,
#endif
#ifndef _SMP_SUPPORT
  nxdevtotty,
#else
  nodevtotty,
#endif
#ifndef _SMP_SUPPORT
  nxselect,
#else
  noselect,
#endif
#ifndef _SMP_SUPPORT
  nxmmap,
#else
  nommap,
#endif
#ifndef _SMP_SUPPORT
  nxstrategy,
#else
  nostrategy,
#endif
  "pciddc",
  (struct bdevsw *) NULL,
  -1
};

#ifndef _SMP_SUPPORT

/*
 * We redefine MOD_DEV because of a lack of braces in its definition in
 * /sys/sys/lkm.h that generates a warning with gcc -Wall.
 */

#ifdef MOD_DEV
#undef MOD_DEV
#endif

#define MOD_DEV(name,devtype,devslot,devp)        \
        MOD_DECL(name);                           \
        static struct lkm_dev name ## _module = { \
                LM_DEV,                           \
                LKM_VERSION,                      \
                #name ## "_mod",                  \
                devslot,                          \
                devtype,                          \
                { (void *)devp }                  \
        }

#endif

MOD_DEV(hsldriver,
	LM_DT_CHAR,
	-1,
	&cdev);

driver_stats_t driver_stats;

/************************************************************/
/* Test of layer PUT */

static int testputbench_ident[MAX_TESTPUTBENCH];
static int hsltestput_layer = -1;

static int hsl2tty_ident[MAX_NODES_SIZE];
static int hsl2tty_size[MAX_NODES_SIZE];

void
hsl2ttysend_cb(caddr_t param, int size)
{
  int s;
  int dest;

  dest = (int) param;

  s = splhigh();
  hsl2tty_size[dest] = size;
  hsl2tty_ident[dest]--;
  wakeup(&hsl2tty_ident[dest]);
  splx(s);
}

void
hsl2ttyrecv_cb(caddr_t param)
{
  psignal((struct proc *) param, (int) SIGIO);
}

/************************************************************/

int
hsl_bench_latence1(int minor, hsl_bench_latence_t *param_lat)
{
  int s;
  lpe_entry_t entry;
  mi_t mi;
  volatile char *target;
  int cpt;
  int global_cnt;
  int res;
  int wait_count;
  int count_limit;

  wait_count = param_lat->wait_count;

  entry.page_length = param_lat->size;
  entry.routing_part = param_lat->dist_node;
  entry.control = LMP_MASK;
  mi = put_get_mi_start(minor,
			hsltestput_layer);
  entry.control |= mi;
  entry.PRSA = (caddr_t) param_lat->dist_addr;
  entry.PLSA = (caddr_t) param_lat->local_addr;

  target = param_lat->local_addr_virt + param_lat->size - 1;

  count_limit = param_lat->count;

  /************************************************************/

  s = splhigh();

  put_flush_lpe(minor);

#ifdef MPC_PERFMON
  __asm__("cli");
  PERFMON_START;
  /* log(LOG_DEBUG, "PERFSTART TVSEC: end.sec:%d end.usec:%d start.sec:%d start.usec:%d\n",
     (int) perfmon_tv_end.tv_sec, (int) perfmon_tv_end.tv_usec,
     (int) perfmon_tv_start.tv_sec, (int) perfmon_tv_start.tv_usec); */
#endif


  if (param_lat->first == TRUE) {
    /* option -1 */

    for (global_cnt = 0; global_cnt < count_limit; global_cnt++) {


/*       log(LOG_DEBUG, "global_cnt = %d\n", global_cnt); */

      *target = '\1';

      /* ATTENTION : 1001 ici signifie qu'on ne flushe les interrupts que toutes
	 les 1001 emissions. Ce flush fait baisser les mesures de perf */
      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      res = put_add_entry(minor, &entry);

      if (res != 0) {
#ifdef MPC_PERFMON
	__asm__("sti");
#endif
	splx(s);
	param_lat->result = global_cnt;
	return res;
      }

      cpt = 0;
      while (*target == '\1') {
	cpt++;
	if (cpt == wait_count) {
#ifdef MPC_PERFMON
	  __asm__("sti");
#endif
	  splx(s);
	  param_lat->result = global_cnt;
	  return EDEADLK;
	}
      }
    }
  } else {
    /* option -2 */

    *target = '\2';

    for (global_cnt = 0; global_cnt < count_limit; global_cnt++) {

/*       log(LOG_DEBUG, "global_cnt = %d\n", global_cnt); */

      cpt = 0;
      while (*target == '\2') {
	cpt++;
	if (cpt == wait_count) {
#ifdef MPC_PERFMON
	  __asm__("sti");
#endif
	  splx(s);
	  param_lat->result = global_cnt;
	  return EDEADLK;
	}
      }

/*       log(LOG_DEBUG, "bench received\n"); */

      *target = '\2';

      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      res = put_add_entry(minor, &entry);
      if (res != 0) {
#ifdef MPC_PERFMON
	__asm__("sti");
#endif
	splx(s);
	param_lat->result = global_cnt;
	return res;
      }
    }
  }

#ifdef MPC_PERFMON
    PERFMON_STOP("BENCH_LATENCE1");
    __asm__("sti");
#endif
  splx(s);

  param_lat->result = global_cnt;
  return 0;
}

/************************************************************/

int
hsl_bench_throughput1(int minor, hsl_bench_throughput_t *param_thr)
{
  int s;
  lpe_entry_t entry;
  mi_t mi;
  volatile char *target;
  int cpt;
  int global_cnt = 0;
  int res;
  int wait_count;
  int count_limit;

  wait_count = param_thr->wait_count;

  entry.routing_part = param_thr->dist_node;
  entry.control = LMP_MASK;
  mi = put_get_mi_start(minor,
			hsltestput_layer);
  entry.control |= mi;
  if (param_thr->first == TRUE) {
    entry.page_length = param_thr->size;
    entry.PRSA = (caddr_t) param_thr->dist_addr;
    entry.PLSA = (caddr_t) param_thr->local_addr;
  } else {
    entry.page_length = 1;
    entry.PRSA = (caddr_t) (param_thr->dist_addr + param_thr->size);
    entry.PLSA = (caddr_t) (param_thr->local_addr + param_thr->size);
  }

  target = param_thr->local_addr_virt + param_thr->size;

  count_limit = param_thr->count;

  /************************************************************/

  s = splhigh();

  put_flush_lpe(minor);

#ifdef MPC_PERFMON
  __asm__("cli");
  PERFMON_START;
#endif

  if (param_thr->first == TRUE) {
    /* option -1 */

    for (global_cnt = 0; global_cnt < count_limit - 1; global_cnt++) {
      /* ATTENTION : 1001 ici signifie qu'on ne flushe les interrupts que toutes
	 les 1001 emissions. Ce flush fait baisser les mesures de perf */
      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      do {
	res = put_add_entry(minor, &entry);
	if (res == EAGAIN)
	  put_flush_lpe(minor);
      } while (res == EAGAIN);

      if (res != 0) {
#ifdef MPC_PERFMON
	__asm__("sti");
#endif
	splx(s);
	param_thr->result = global_cnt;
	return res;
      }
    }

    entry.page_length++;
    *target = '\1';
    do {
      res = put_add_entry(minor, &entry);
      if (res == EAGAIN)
	put_flush_lpe(minor);
    } while (res == EAGAIN);
    if (res != 0) {
#ifdef MPC_PERFMON
      __asm__("sti");
#endif
      splx(s);
      param_thr->result = global_cnt;
      return res;
    }

    cpt = 0;
    while (*target == '\1') {
      cpt++;
      if (cpt == wait_count) {
#ifdef MPC_PERFMON
	__asm__("sti");
#endif
	splx(s);
	param_thr->result = global_cnt;
	return EDEADLK;
      }
    }

  } else {
    /* option -2 */

    *target = '\2';
    cpt = 0;
    while (*target == '\2') {
      cpt++;
      if (cpt == wait_count) {
#ifdef MPC_PERFMON
	__asm__("sti");
#endif
	splx(s);
	param_thr->result = global_cnt;
	return EDEADLK;
      }
    }

    *target = '\2';
    res = put_add_entry(minor, &entry);
    if (res != 0) {
#ifdef MPC_PERFMON
      __asm__("sti");
#endif
      splx(s);
      param_thr->result = global_cnt;
      return res;
    }
  }

#ifdef MPC_PERFMON
    PERFMON_STOP("BENCH_THROUGHPUT1");
    __asm__("sti");
#endif
  splx(s);

  param_thr->result = global_cnt;
  return 0;
}


/*
 ************************************************************
 * hsltestputrecv_irqsent(minor, mi, entry) : IRQ handler for
 * the test/bench utility.
 * minor : board number.
 * mi    : mi associated with this interruption.
 * entry : the entry that has just left the LPE.
 ************************************************************
 */

void
hsltestputrecv_irqsent(int minor,
		       mi_t mi,
		       lpe_entry_t entry)
{
  TRY("hsltestputrecv_irqsent")
#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "hsltestputrecv_irqsent: mi=0x%x\n",
      minor);

  log(LOG_DEBUG,
      "hsltestputrecv_irqsent: entry.PRSA=0x%x\n",
      (u_int) entry.PRSA);

  log(LOG_DEBUG,
      "hsltestputrecv_irqsent: entry.PLSA=0x%x\n",
      (u_int) entry.PLSA);
#endif
}


/*
 ************************************************************
 * hsltestputrecv_irqreceived(minor, mi, entry) : IRQ handler for
 * the test/bench utility.
 * minor : board number.
 * mi    : mi associated with this interruption.
 * data1 : opaque data.
 * data2 : opaque data.
 ************************************************************
 */

void
hsltestputrecv_irqreceived(int minor,
			   mi_t mi,
			   u_long data1,
			   u_long data2)
{
  int s;

  TRY("hsltestputrecv_irqreceived")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "hsltestputrecv_irqreceived: mi=0x%x\n",
      (u_int) mi);

  log(LOG_DEBUG,
      "hsltestputrecv_irqreceived: entry.data1=0x%x\n",
      (u_int) data1);

  log(LOG_DEBUG,
      "hsltestputrecv_irqreceived: entry.data2=0x%x\n",
      (u_int) data2);
#endif

  s = splhigh();

  /* log(LOG_ERR, "%x %x %x\n", mi, put_get_mi_start(minor, hsltestput_layer),
     testputbench_ident); */

  testputbench_ident[mi - put_get_mi_start(minor, hsltestput_layer)]++;
  wakeup(testputbench_ident + (mi - put_get_mi_start(minor, hsltestput_layer)));

  splx(s);
}

/************************************************************/

static int txt_offset;
static char txt_info[10000];

#define PRINT_DEV(X) \
  sprintf(txt_info + strlen(txt_info), X); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define PRINT_DEV_1(X, Y) \
  sprintf(txt_info + strlen(txt_info), X, Y); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define PRINT_DEV_5(X, Y, Z, T, U, V) \
  sprintf(txt_info + strlen(txt_info), X, Y, Z, T, U, V); \
  if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { \
    sprintf(txt_info + strlen(txt_info), \
	    "--TRUNCATED--\n"); \
    return; \
  }

#define FORMAT_PTR   1
#define FORMAT_SHORT 2
#define FORMAT_INT   3


/*
 ************************************************************
 * format(type, value) : format a long.
 * type  : type of format.
 * value : value to format.
 * 
 * return value : resulting string.
 ************************************************************
 */

static char *
format(type, value)
       int  type;
       long value;
{
  char tmp[256];
  int i;
  static char strings[10][256];
  static int current = 0;

  if (current == 10)
    current = 0;

  for (i = 0;
       i < sizeof strings[current];
       i++)
    strings[current][i] = ' ';

  switch (type) {
  case FORMAT_PTR:
    sprintf(tmp,
	    "%p",
	    (caddr_t) value);
    strcpy(strings[current] + 10 - strlen(tmp),
	 tmp);
    break;

  case FORMAT_SHORT:
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 4 - strlen(tmp),
	 tmp);
    break;

  case FORMAT_INT:
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 8 - strlen(tmp),
	 tmp);
    break;

  default:
    sprintf(strings[current],
	    "ERROR");
  }

  return strings[current++];
}

/* NOT USED in this driver.
static char *
format_str(size, value)
          int   size;
          char *value;
{
  char tmp[256];
  int i;
  static char strings[10][256];
  static int current = 0;

  if (current == 10)
    current = 0;

  for (i = 0;
       i < sizeof strings[current];
       i++)
    strings[current][i] = ' ';

  sprintf(tmp,
	  "%s",
	  value);
  strcpy(strings[current] + size - strlen(tmp),
	 tmp);

  return strings[current++];
}
*/



/*
 ************************************************************
 * driver_update() : log internal stats onto the console.
 ************************************************************
 */

static void
driver_update()
{
#ifdef DEBUG_HSL
  /*  log(LOG_DEBUG,
      "cmem_update()\n"); */
#endif

  txt_info[0] = 0;

  PRINT_DEV("/dev/hsl : kernel drivers and protocols for MPC parallel computer\n\n");

#ifdef MPC_STATS

  PRINT_DEV("***** SLR/V - SCP/V\n");
  PRINT_DEV_1("calls to send()           : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_slrpv_send));
  PRINT_DEV_1("calls to recv()           : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_slrpv_recv));
  PRINT_DEV("***** SLR/P - SCP/V\n");
  PRINT_DEV_1("calls to send()           : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_slrpp_send));
  PRINT_DEV_1("calls to recv()           : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_slrpp_recv));
  PRINT_DEV_1("calls to ddslrp_timeout() : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_timeout));
  PRINT_DEV_1("timeouts from timeouts    : %s\n",
	      format(FORMAT_INT,
		     driver_stats.timeout_timeout));
  PRINT_DEV("***** PUT\n");
  PRINT_DEV_1("calls to put_add_entry()  : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_put_add_entry));
  PRINT_DEV_1("hardware interrupts       : %s\n",
	      format(FORMAT_INT,
		     driver_stats.calls_put_interrupt));

  PRINT_DEV_1("unattended interrupts     : %s\n",
	      format(FORMAT_INT,
		     driver_stats.unattended_interrupts));

#else

  PRINT_DEV("MPC-OS compiled without statistics code\n");

#endif

}

/************************************************************/

int event_ident;

trace_var_t trace_var;

static int trace_events_low;
static int trace_events_high;
static int trace_events_index;

trace_event_t trace_events[MAX_EVENTS];


/*
 ************************************************************
 * init_events() : initialize event processing.
 ************************************************************
 */

void
init_events()
{
  trace_events_low = trace_events_high = trace_events_index = 0;
}


/*
 ************************************************************
 * add_event(type, entry, event) : insert an event in the event queue.
 * type  : type of event (integer/string).
 * entry : event number.
 * event : event value.
 ************************************************************
 */

void
add_event(type, entry, event)
     trace_type_t       type;
     trace_type_entry_t entry;
     trace_t            event;
{
  int i/*, s*/;

  /* s = splhigh(); */

  i = ((trace_events_high + 1) == MAX_EVENTS) ? 0 : trace_events_high + 1;
  if (i == trace_events_low) {
    trace_events_index++;

    /* splx(s); */
    return;
  }

  trace_events[trace_events_high].type       = type;
  trace_events[trace_events_high].entry_type = entry;
  trace_events[trace_events_high].index      = trace_events_index;
  trace_events[trace_events_high].trace      = event;

  microtime(&trace_events[trace_events_high].tv);

  trace_events_high = i;
  trace_events_index++;

  wakeup(&event_ident);

  /* splx(s); */
}


/*
 ************************************************************
 * add_event(type, entry, event) : insert a string in the event queue.
 * str : string to insert.
 ************************************************************
 */

void
add_event_string(str)
     char *str;
{
  trace_t event;
  int s;

  s = splhigh();

  if (perfmon_invalid) {
    perfmon_invalid = 0;
    add_event_string(">>>INVALID<<<");
  }

  strncpy(event.str, str, MAX_EVENTS_STRING_SZ - 1);
  event.str[MAX_EVENTS_STRING_SZ - 1] = 0;
  add_event(TRACE_STR, TRACE_NO_TYPE, event);

  perfmon_invalid = 0;

  splx(s);
}


/*
 ************************************************************
 * add_event_tv(comment) : insert a timeval string in the event queue.
 * comment : string to insert in the event queue.
 ************************************************************
 */

void
add_event_tv(comment)
             char *comment;
{
  u_int diff;
  char str[MAX_EVENTS_STRING_SZ];

  if (!perfmon_invalid) {

    /*
      log(LOG_DEBUG, "TVSEC: end.sec:%d end.usec:%d start.sec:%d start.usec:%d\n", 
      (int) perfmon_tv_end.tv_sec, (int) perfmon_tv_end.tv_usec,
      (int) perfmon_tv_start.tv_sec, (int) perfmon_tv_start.tv_usec);
    */

    diff = perfmon_tv_end.tv_sec * 1000000 + perfmon_tv_end.tv_usec -
      (perfmon_tv_start.tv_sec * 1000000 + perfmon_tv_start.tv_usec);
    sprintf(str, "DELTA TIME : %d microsec - %s", diff, comment);
    add_event_string(str);
  }
}


/*
 ************************************************************
 * get_event() : get an event from the event queue.
 *
 * return value : an event or NULL if the queue is empty.
 ************************************************************
 */

trace_event_t *
get_event()
{
  int s;
  trace_event_t *result;

  s = splhigh();

  if (trace_events_high == trace_events_low) {
    splx(s);
    return NULL;
  }

  result = trace_events + trace_events_low;

  trace_events_low++;
  if (trace_events_low == MAX_EVENTS) trace_events_low = 0;

  splx(s);
  return result;
}


/************************************************************/

/*
 ************************************************************
 * dev_open() : handler for the open operation on the driver.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_open()
{
  TRY("dev_open")

#ifdef DEBUG_HSL
    /*  log(LOG_DEBUG,
      "dev_open\n"); */
#endif

  txt_offset = 0;
  driver_update();

  open_count++;

  return 0;
}


/*
 ************************************************************
 * dev_close() : handler for the close operation on the driver.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_close()
{
  TRY("dev_close")

  txt_offset = 0;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_close\n");
#endif

  open_count--;
  return 0;
}


/*
 ************************************************************
 * dev_read() : handler for the read operation on the driver.
 * dev  : device.
 * uio  : read vector.
 * flag : flags.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_read(dev, uio, flag)
         dev_t dev;
         struct uio *uio;
         int flag;
{
  int res;
  int len;

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "dev_read\n");
#endif

  len = uio->uio_iov->iov_len;
  len = MIN(len, strlen(txt_info) - txt_offset);
  if (!len) return 0;

  res = uiomove(txt_info + txt_offset, len, uio);
  txt_offset += len;

  return res;
}


/************************************************************/

/*
 ************************************************************
 * vmflags2str(flagsstr, flags) : convert some VM flags to a string.
 * flagsstr : result string.
 * flags    : VM flags.
 ************************************************************
 */

void vmflags2str(flagsstr, flags)
                 char *flagsstr;
                 u_short flags;
{
  flagsstr[0] = 0;
#ifndef _SMP_SUPPORT
  if (flags & OBJ_CANPERSIST)   strcat(flagsstr, " CANPERSIST");
#endif
  if (flags & OBJ_ACTIVE)       strcat(flagsstr, " ACTIVE");
  if (flags & OBJ_DEAD)         strcat(flagsstr, " DEAD");
  if (flags & OBJ_PIPWNT)       strcat(flagsstr, " PIPWNT");
  if (flags & OBJ_WRITEABLE)    strcat(flagsstr, " WRITEABLE");
  if (flags & OBJ_MIGHTBEDIRTY) strcat(flagsstr, " MIGHTBEDIRTY");
  if (flags & OBJ_CLEANING)     strcat(flagsstr, " CLEANING");
#ifndef _SMP_SUPPORT
  if (flags & OBJ_VFS_REF)      strcat(flagsstr, " VFS_REF");
  if (flags & OBJ_VNODE_GONE)   strcat(flagsstr, " VNODE_GONE");
#endif
  if (flags & OBJ_NORMAL)       strcat(flagsstr, " NORMAL");
  if (flags & OBJ_SEQUENTIAL)   strcat(flagsstr, " SEQUENTIAL");
  if (flags & OBJ_RANDOM)       strcat(flagsstr, " RANDOM");
}

/************************************************************/


#ifndef _SMP_SUPPORT
#ifdef WITH_STABS_PCIBUS_SET
extern struct linker_set _pcibus_set;
#endif
#endif


/*
 ************************************************************
 * getpcibus() : get a pcibus structure associated with the FastHSL board.
 * 
 * return value : the pcibus structure or NULL if no board found.
 ************************************************************
 */

#ifndef _SMP_SUPPORT
struct pcibus *
getpcibus(void)
{
#ifdef WITH_STABS_PCIBUS_SET
  pcibus_set_p = &_pcibus_set;
#else
  if (pcibus_set_p == NULL) {
    log(LOG_ERR,
	"You should run init_pcibus before using the PCI API\n");
    return NULL;
  }
#endif

  switch (pcibus_set_p->ls_length) {
  case 0:
    log(LOG_ERR,
	"No PCI bus detected !\n");
    return NULL;
    break; /* NOTREACHED */

  case 1:
    break;

  default:
    log(LOG_ERR,
	"No support for more than one PCI bus on the board\n");
    break; /* NOTREACHED */
  };
    
  return (struct pcibus *) pcibus_set_p->ls_items[0];
}
#endif


static void Callback(caddr_t param, int size)
{
 cpt2++;

log(LOG_DEBUG,"Appel Callback %d\n",cpt);
(*param)++;
wakeup(param);
}



/*
 ************************************************************
 * dev_ioctl() : handler for the ioctl operation on the driver.
 * dev  : device.
 * com  : type of IOCTL.
 * data : R/W data passed from/to the process.
 * flag : flags.
 * p    : proc struct of the calling process.
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
dev_ioctl(dev, com, data, flag, p)
     dev_t dev;
     int com;
     caddr_t data;
     int flag;
     struct proc *p;
{
  struct _virtualRegion *vz;
  struct _physicalWrite *pw2;
  physical_read_t *pr;
  physical_write_t *pw;
  pagedescr_t pagedescr[10];
  int status, ss;
#ifndef _SMP_SUPPORT
  pcici_t tag;
#else
  pcicfgregs probe;
#endif
  trace_event_t *event_p;
  int s, tmp, res, i, j;
  node_t node;

  TRY("dev_ioctl")

#ifdef DEBUG_HSL
/*     log(LOG_DEBUG, "dev_ioctl\n"); */
#endif

  switch (com) {
  case HSLTESTSLR:
    TRY("HSLTESTSLR")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLR\n");
#endif

    pagedescr[0].addr = *(caddr_t *) data;
    pagedescr[0].size = 2;
    pagedescr[1].addr = 2 + *(caddr_t *) data;
    pagedescr[1].size = 1;
    /* dest : noeud 1; channel : 3; 1 page de 1 octet */
    res = slrpp_send(1, 3, pagedescr, 2, NULL, NULL, p);
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTSLR2:
    TRY("HSLTESTSLR2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLR2\n");
#endif

    pagedescr[0].addr = *(caddr_t *) data;
    pagedescr[0].size = 1;
    pagedescr[1].addr = 1 + *(caddr_t *) data;
    pagedescr[1].size = 2;
    /* dest : noeud 0; channel : 3; 1 page de 1 octet */
    res = slrpp_recv(0, 3, pagedescr, 2, NULL, NULL, p);
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTSLRV:
    TRY("HSLTESTSLRV")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLRV\n");
#endif

    /* dest : noeud 1; channel : 3; 1 page de 1 octet */
    status=0;
    cpt++;
    res = slrpv_send(1, 3, *(caddr_t *) data, 100000, Callback, (caddr_t) &status, p);
    if (res < 0)
      {
	log(LOG_DEBUG,"Res Send <0\n");
	return res;
      }

    ss=splhigh();
    log(LOG_DEBUG,"Attendre l'envoi %d\n",cpt);
    while(!status) tsleep(&status, HSLPRI,"DATASENT", 0);
    status--;
    splx(ss);
    log(LOG_DEBUG,"Fin de l'attente\n");
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTSLRV2:
    TRY("HSLTESTSLRV2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLRV2\n");
#endif

    /* dest : noeud 0; channel : 3; 1 page de 1 octet */
/*A REMETTRE : (test loopback)     res = slrpv_recv(0, 3, *(caddr_t *) data, 5000, NULL, NULL, p); */
    status=0;
    cpt++;
    res = slrpv_recv(0, 3, *(caddr_t *) data, 100000, Callback, &status, p);
    if (res < 0)
      {
	log(LOG_DEBUG,"Res Recv <0\n");
	return res;
      }
    ss=splhigh();
    log(LOG_DEBUG,"Attendre la reception %d\n",cpt);
    while(!status) tsleep(&status, HSLPRI,"DATARECV", 0);
    status--;
    splx(ss);
    log(LOG_DEBUG,"Fin de l'attente\n");
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/
  case HSLTESTSLRVPROT:
    TRY("HSLTESTSLRVPROT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLRVPROT\n");
#endif

    /* dest : noeud 1; channel : 3; 1 page de 1 octet */
    res = slrpv_send_prot(1, 3,
			  (*(struct _virtualRegion *) data).start,
			  (*(struct _virtualRegion *) data).size,
			  NULL, NULL, p);
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTSLRV2PROT:
    TRY("HSLTESTSLRV2PROT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTSLRV2PROT\n");
#endif

    /* dest : noeud 0; channel : 3; 1 page de 1 octet */
    res = slrpv_recv_prot(0, 3,
			  (*(struct _virtualRegion *) data).start,
			  (*(struct _virtualRegion *) data).size,
			  NULL, NULL, p);
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTEST:
    TRY("HSLTEST")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTEST\n");
#endif

    wakeup(&pending_send_free_ident);

    /*
    if (!contig_space) init_slr(0);
    else end_slr();
    */
    /*    *((int *) data) = 1;*/
    break;

    /************************************************************/

  case HSLVIRT2PHYSTST:
    TRY("HSLVIRT2PHYSTST")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLVIRT2PHYSTST\n");
#endif

    vz = (struct _virtualRegion *) data;

    log(LOG_DEBUG, "0x%x\n", vtophys(vz->start));

    break;

    /************************************************************/

  case HSLTESTPUTRECV1:
    TRY("HSLTESTPUTRECV1")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTRECV1\n");
#endif

    {
      int res;

      if (hsltestput_layer != -1) {
	res = put_unregister_SAP(minor(dev),
				 hsltestput_layer);
	hsltestput_layer = -1;
	if (res)
	  return res;
      }

      hsltestput_layer = put_register_SAP(minor(dev),
					  hsltestputrecv_irqsent,
					  hsltestputrecv_irqreceived);

      if (hsltestput_layer < 0)
	return ENOMEM;

      res = put_attach_mi_range(minor(dev),
				hsltestput_layer,
				MAX_TESTPUTBENCH);

      if (res)
	return res;

      *((mi_t *) data) = put_get_mi_start(minor(dev),
					  hsltestput_layer);
      return 0;
    }
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTPUTRECV2:
    TRY("HSLTESTPUTRECV2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTRECV2\n");
#endif

    {
      int res;

      if (hsltestput_layer != -1) {
	res = put_unregister_SAP(minor(dev),
				 hsltestput_layer);
	hsltestput_layer = -1;
	if (res)
	  return res;
      }
    }
    break;

    /************************************************************/

  case HSLTESTPUTSEND0:
    TRY("HSLTESTPUTSEND0")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTSEND0\n");
#endif

    {
      int res;
      mi_t mi;
      lpe_entry_t entry;

      entry = *(lpe_entry_t *) data;

      if (hsltestput_layer < 0)
	return ENOMEM;

      mi = put_get_mi_start(minor(dev),
			    hsltestput_layer);

      entry.control |= mi;

      /* INCREASE "i < 1" to bench layer PUT */
      /*
      {
	int s;
	register int i;
	register int bad;

	__asm__("cli");
	s = splhigh();

	PERFMON_START;

	bad = 0;

	for (i = 0; i < 1; i++) {
	  res = put_add_entry(minor(dev), &entry);
	  if (res != 0) bad = 1;
	}

	if (bad == 0)
	  PERFMON_STOP("BENCH PUT");

	__asm__("sti");
	splx(s);
      }
      */
      res = put_add_entry(minor(dev), &entry);

      if (res)
	return res;

      return 0;
    }
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTPUTSEND1:
    TRY("HSLTESTPUTSEND1")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTSEND1\n");
#endif

    {
      int res;
      lpe_entry_t entry;

      entry = *(lpe_entry_t *) data;

      if (hsltestput_layer < 0)
	return ENOMEM;

      /* INCREASE "i < 1" to bench layer PUT */
      /*
      {
	int s;
	register int i;
	register int bad;

	__asm__("cli");
	s = splhigh();

	PERFMON_START;

	bad = 0;

	for (i = 0; i < 1; i++) {
	  res = put_add_entry(minor(dev), &entry);
	  if (res != 0) bad = 1;
	}

	if (bad == 0)
	  PERFMON_STOP("BENCH PUT");

	__asm__("sti");
	splx(s);
      }
      */
      res = put_add_entry(minor(dev), &entry);

      if (res)
	return res;

      return 0;
    }
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLTESTPUTSEND2:
    TRY("HSLTESTPUTSEND2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTSEND2\n");
#endif

    {
    }
    break;

    /************************************************************/

  case HSLTESTPUTBENCH1:
    TRY("HSLTESTPUTBENCH1")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTBENCH1\n");
#endif

    s = splhigh();
  restart:
    if (!testputbench_ident[*(int *) data]) {
#ifdef MPC_PERFMON
      perfmon_invalid = 1;
#endif
      res = tsleep(&testputbench_ident[*(int *) data],
		   HSLPRI | PCATCH, "BENCH1", NULL);
      if (!res)
	goto restart;
      splx(s);

      /*
       * Don't want to retry the system call even if a signal with
       * the SA_RESTART flag interrupted it
       */
      if (res == ERESTART) res = EINTR;

      return res;
    }
    testputbench_ident[*(int *) data]--;
    splx(s);
    break;

    /************************************************************/

  case HSLTESTPUTBENCH2:
    TRY("HSLTESTPUTBENCH2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLTESTPUTBENCH2\n");
#endif

    testputbench_ident[*(int *) data] = 0;
    *(int * ) data = put_get_mi_start(minor(dev),
				      hsltestput_layer) + *(int *) data;
    break;

    /************************************************************/

  case HSL2TTYINIT:

    TRY("HSL2TTYINIT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSL2TTYINIT\n");
#endif
/* log(LOG_DEBUG, "IOCTL HSL2TTYINIT\n"); */

    hsl2tty_ident[*(int *) data] = 0;
    break;

    /************************************************************/

  case HSL2TTYSEND:
    TRY("HSL2TTYSEND")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSL2TTYSEND\n");
#endif
/* log(LOG_DEBUG, "IOCTL HSL2TTYSEND\n"); */

    s = splhigh();

#if 0
    {
      char ch[256];
      sprintf(ch, "IOCTL HSL2TTYSEND seq=%x",
	      send_seq[NODE2NODEINDEX(((hsl2tty_t *) data)->dest, my_node)]
	              [0].seq);
      add_event_string(ch);
    }
#endif

    if (slrpv_cansend(((hsl2tty_t *) data)->dest, 0) == FALSE) {
      log(LOG_ERR,
	  "HSL2TTYSEND error 1\n");
      splx(s);
      return EWOULDBLOCK;
    }

    hsl2tty_ident[((hsl2tty_t *) data)->dest]++;

    splx(s);

    res = slrpv_send_prot(((hsl2tty_t *) data)->dest,
			  0,
			  ((hsl2tty_t *) data)->addr,
			  ((hsl2tty_t *) data)->size,
			  hsl2ttysend_cb,
			  (caddr_t) (int) (((hsl2tty_t *) data)->dest),
			  p);
    if (res) return res;

    s = splhigh();

  ttyrestart:

    /*
     * Uncommenting ERESTART..EINTR seems to leed to an infinite loop !
     * We choose to solve the problem by commenting PCATCH 
     */
    if (hsl2tty_ident[((hsl2tty_t *) data)->dest] > 0) {
      res = tsleep(&hsl2tty_ident[((hsl2tty_t *) data)->dest],
		   HSLPRI/* | PCATCH*/, "HSL2TTY", NULL);
      if (!res/* || res == ERESTART || res == EINTR*/) goto ttyrestart;
      else {
	splx(s);
	return res;
      }
    }

    ((hsl2tty_t *) data)->size = hsl2tty_size[((hsl2tty_t *) data)->dest];
    splx(s);

    break;

    /************************************************************/

  case HSL2TTYCANSEND:
    TRY("HSL2TTYCANSEND")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSL2TTYCANSEND\n");
#endif
/* log(LOG_DEBUG, "IOCTL HSL2TTYCANSEND\n"); */

    node = *(int *) data;

#if 0
    {
      char ch[256];
      sprintf(ch, "IOCTL HSL2TTYCANSEND seq=%x",
	      send_seq[NODE2NODEINDEX(node, my_node)][0].seq);
      add_event_string(ch);
    }
#endif

    *(int *) data = slrpv_cansend(node, 0);
    break;

    /************************************************************/

  case HSL2TTYRECV:
    TRY("HSL2TTYRECV")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSL2TTYRECV\n");
#endif
/* log(LOG_DEBUG, "IOCTL HSL2TTYRECV\n"); */

    res = slrpv_recv_prot(((hsl2tty_t *) data)->dest,
			  0,
			  ((hsl2tty_t *) data)->addr,
			  ((hsl2tty_t *) data)->size,
			  hsl2ttyrecv_cb,
			  (caddr_t) p,
			  p);
    if (res) return res;
    break;

    /************************************************************/

#ifdef _WITH_MONITOR
  case HSLGETDTP:
    TRY("HSLGETDTP");

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL PUTGETDTP");
#endif

    return put_get_dtp(*(pnode_t *) data);
    /* NOTREACHED */
    break;
#endif

    /************************************************************/

  case HSLVIRT2PHYS:
    TRY("HSLVIRT2PHYS")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLVIRT2PHYS\n");
#endif

    *(caddr_t *) data = (caddr_t) vtophys(*(caddr_t *) data);
    break;

    /************************************************************/

  case HSLADDLPE:
    TRY("HSLADDLPE")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLADDLPE\n");
#endif

    res = put_add_entry(minor(dev), (lpe_entry_t *) data);
    log(LOG_DEBUG, "HSLADDLPE : res = %d\n", res);
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLGETLPE:
    TRY("HSLGETLPE")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLGETLPE\n");
#endif

    s = splhigh();

    for (ever) {
      res = put_get_entry(minor(dev), (lpe_entry_t *) data);
      if (res != EAGAIN) {
	splx(s);
	return res;
      }

#ifdef MPC_PERFMON
      perfmon_invalid = 1;
#endif
      res = tsleep(&hslgetlpe_ident, HSLPRI | PCATCH, "GETLPE", NULL);
      if (res) {
	/*
	 * Don't want to retry the system call even if a signal with
	 * the SA_RESTART flag interrupted it
	 */
	if (res == ERESTART) res = EINTR;

	splx(s);
	return res;
      }
    }

    /* NOTREACHED */
    splx(s);
    break;

    /************************************************************/

  case HSLFLUSHLPE:
    TRY("HSLFLUSHLPE")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLFLUSHLPE\n");
#endif

    return put_flush_entry(minor(dev));
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSIMINT:
    TRY("HSLSIMINT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLSIMINT\n");
#endif

    return put_simulate_interrupt(minor(dev), *(lpe_entry_t *) data);
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSETCONFIG:
    TRY("HSLSETCONFIG")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLSETCONFIG\n");
#endif

    return set_config_slr(*(slr_config_t *) data);
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLREADPHYS:
    TRY("HSLREADPHYS")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLREADPHYS\n");
#endif

    s = splhigh();

    pr = (physical_read_t *) data;

    if (!pr->len) return 0;
    if (trunc_page((unsigned) pr->src) != trunc_page((unsigned) (pr->src + pr->len - 1))) {
      splx(s);
      return EINVAL;
    }

    *(int *) CMAP1 = PG_V | PG_RW | trunc_page((unsigned) pr->src);

    tmp = *(int *) (CADDR1 + ((int) pr->src & PAGE_MASK));

    res = copyout((char *) (CADDR1 + ((int) pr->src & PAGE_MASK)),
		  pr->data,
		  pr->len);

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    if (res)
      return res;
    break;

    /************************************************************/

  case HSLWRITEPHYS:
    TRY("HSLWRITEPHYS")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLWRITEPHYS\n");
#endif

    s = splhigh();

    pw = (physical_write_t *) data;

    if (!pw->len) return 0;
    if (trunc_page((unsigned) pw->dst) != trunc_page((unsigned) (pw->dst + pw->len - 1))) {
      splx(s);
      return EINVAL;
    }

    *(int *) CMAP1 = PG_V | PG_RW | trunc_page((unsigned) pw->dst);

    res = copyin(pw->data,
		 (char *) (CADDR1 + ((int) pw->dst & PAGE_MASK)),
		 pw->len);

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    if (res)
      return res;
    break;

    /************************************************************/

  case HSLWRITEPHYS2:
    TRY("HSLWRITEPHYS2")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLWRITEPHYS\n");
#endif

    s = splhigh();

    pw2 = (struct _physicalWrite *) data;

    *(int *) CMAP1 = PG_V | PG_RW | trunc_page((unsigned) pw2->addr);

    tmp = *(int *)(CADDR1 + ((int) pw2->addr & PAGE_MASK));
    pw2->value = tmp;

/*    log(LOG_DEBUG, "*value = 0x%x \n", tmp);*/

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    break;

    /************************************************************/

  case HSLPUTINIT:
    TRY("HSLPUTINIT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLPUTINIT\n");
#endif

    res = put_init(minor(dev),
		   ((hsl_put_init_t *) data)->length,
		   ((hsl_put_init_t *) data)->node,
		   ((hsl_put_init_t *) data)->routing_table);

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "hslputinit : res = %d\n", res);
#endif
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSLRPINIT:
    TRY("HSLSLRPINIT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLSLRPINIT\n");
#endif

    res = init_slr(minor(dev));
    if (!res) *(caddr_t *) data = contig_space_phys;
    else *(caddr_t *) data = NULL;

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "ioctl hslslrpinit: contig_space = [virt: 0x%x] [phys: 0x%x]\n", contig_space, (u_int) contig_space_phys);
#endif

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLPUTEND:
    TRY("HSLPUTEND")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLPUTEND\n");
#endif

    res = put_end(minor(dev));
    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSLRPEND:
    TRY("HSLSLRPEND")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLPUTEND\n");
#endif

    end_slr();
    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLGETNODE:
    TRY("HSLGETNODE")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLGETNODE\n");
#endif

    *(node_t *) data = put_get_node(minor(dev));
    break;

    /************************************************************/

  case HSLGETOPTCONTIGMEM:
    TRY("HSLGETOPTCONTIGMEM")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLGETOPTCONTIGMEM\n");
#endif

    ((opt_contig_mem_t *) data)->ptr  = (caddr_t) opt_contig_space;
    ((opt_contig_mem_t *) data)->size = (size_t)  opt_contig_size;
    ((opt_contig_mem_t *) data)->phys = (caddr_t) opt_contig_space_phys;
    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case PCIREADCONF:
    TRY("PCIREADCONF")

#ifdef DEBUG_HSL
/*       log(LOG_DEBUG, "IOCTL PCIREADCONF\n"); */
#endif

#ifndef _SMP_SUPPORT
    if (getpcibus() == NULL)
      return ENODEV;
#endif

    s = splhigh();
#ifndef _SMP_SUPPORT
    tag = getpcibus()->pb_tag(((mpc_pci_conf_t *) data)->bus,
			      ((mpc_pci_conf_t *) data)->device,
			      ((mpc_pci_conf_t *) data)->func);

    ((mpc_pci_conf_t *) data)->data = 
      getpcibus()->pb_read(tag,
			   ((mpc_pci_conf_t *) data)->reg);
#else
    probe.bus  = ((mpc_pci_conf_t *) data)->bus;
    probe.slot = ((mpc_pci_conf_t *) data)->device;
    probe.func = ((mpc_pci_conf_t *) data)->func;

    ((mpc_pci_conf_t *) data)->data = 
      pci_cfgread(&probe, ((mpc_pci_conf_t *) data)->reg, 4);
#endif
    splx(s);

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case PCIWRITECONF:
    TRY("PCIWRITECONF")

#ifdef DEBUG_HSL
      /*      log(LOG_DEBUG, "IOCTL PCIWRITECONF\n");*/
#endif

#ifndef _SMP_SUPPORT
    if (getpcibus() == NULL)
      return ENODEV;
#endif

    s = splhigh();
#ifndef _SMP_SUPPORT
    tag = getpcibus()->pb_tag(((mpc_pci_conf_t *) data)->bus,
			      ((mpc_pci_conf_t *) data)->device,
			      ((mpc_pci_conf_t *) data)->func);

    getpcibus()->pb_write(tag,
			  ((mpc_pci_conf_t *) data)->reg,
			  ((mpc_pci_conf_t *) data)->data);
#else
    probe.bus  = ((mpc_pci_conf_t *) data)->bus;
    probe.slot = ((mpc_pci_conf_t *) data)->device;
    probe.func = ((mpc_pci_conf_t *) data)->func;
    
    pci_cfgwrite(&probe, ((mpc_pci_conf_t *) data)->reg,
		 ((mpc_pci_conf_t *) data)->data, 4);
#endif
    splx(s);

    return 0;

    /************************************************************/

  case HSLGETEVENT:
    TRY("HSLGETEVENT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLGETEVENT\n");
#endif

    s = splhigh();

  try_event:
    event_p = get_event();
    if (!event_p) {
#ifdef MPC_PERFMON
      perfmon_invalid = 1;
#endif
      res = tsleep(&event_ident,
		   HSLPRI | PCATCH,
		   "GETEVENT",
		   NULL);
      if (!res)
	goto try_event; /* TRY AGAIN */

      splx(s);

      /*
       * Don't want to retry the system call even if a signal with
       * the SA_RESTART flag interrupted it
       */
      if (res == ERESTART) res = EINTR;

      return res;
    }

    *(trace_event_t *) data = *event_p;

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLVMINFO:
    TRY("HSLVMINFO")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLVMINFO\n");
#endif

    {
      int i = 0;
      pid_t pid;
      struct proc *pr;
      struct vm_map_entry *current_entry;
      char protstr[4], maxprotstr[4];
      char flagsstr[256];

      pid = *(pid_t *) data;
      pr = pfind(pid);
      log(LOG_INFO, "Virtual memory map of process %d (proc : %p)\n", pid, pr);
      if (!pr) return EINVAL;

      log(LOG_INFO, "text %p (size: %d pages)\n", pr->p_vmspace->vm_taddr,
	  pr->p_vmspace->vm_tsize);
      log(LOG_INFO, "data %p (size: %d pages)\n", pr->p_vmspace->vm_daddr,
	  pr->p_vmspace->vm_dsize);
      log(LOG_INFO, "stack size: %d pages\n", pr->p_vmspace->vm_ssize);

      /* sys/vm/vm_map.h */
      log(LOG_INFO, "nentries in vm_map : %d\n",
	  pr->p_vmspace->vm_map.nentries);
      log(LOG_INFO, "vm_map virtual size : 0x%x\n",
	  pr->p_vmspace->vm_map.size);

      current_entry = &pr->p_vmspace->vm_map.header;

      for (i = 0;
	   (i < 1 + pr->p_vmspace->vm_map.nentries) && current_entry;
	   i++, (current_entry = current_entry->next)) {
	protstr[0] = (current_entry->protection & VM_PROT_READ) ?
	  'R' : ' ';
	protstr[1] = (current_entry->protection & VM_PROT_WRITE) ?
	  'W' : ' ';
	protstr[2] = (current_entry->protection & VM_PROT_EXECUTE) ?
	  'X' : ' ';
	protstr[3] = 0;
	maxprotstr[0] = (current_entry->max_protection & VM_PROT_READ) ?
	  'R' : ' ';
	maxprotstr[1] = (current_entry->max_protection & VM_PROT_WRITE) ?
	  'W' : ' ';
	maxprotstr[2] = (current_entry->max_protection & VM_PROT_EXECUTE) ?
	  'X' : ' ';
	maxprotstr[3] = 0;
	log(LOG_INFO, "---\n");
	log(LOG_INFO, "vm_map_entry %d : %p -> %p / prot[%s] - max[%s]\n", i,
	    (caddr_t) current_entry->start,
	    (caddr_t) current_entry->end, protstr, maxprotstr);

	flagsstr[0] = 0;
	if (current_entry->eflags & MAP_ENTRY_IS_A_MAP)
	  strcpy(flagsstr, " IS_A_MAP");
	if (current_entry->eflags & MAP_ENTRY_IS_SUB_MAP)
	  strcat(flagsstr, " IS_SUBMAP");
	if (current_entry->eflags & MAP_ENTRY_IS_SUB_MAP)
	  strcat(flagsstr, " COPY_ON_WRITE");
	if (current_entry->eflags & MAP_ENTRY_NEEDS_COPY)
	  strcat(flagsstr, " NEEDS_COPY");
	if (current_entry->eflags & MAP_ENTRY_NOFAULT)
	  strcat(flagsstr, " NO_FAULT");
	if (current_entry->eflags & MAP_ENTRY_USER_WIRED)
	  strcat(flagsstr, " USER_WIRED");
	/* /usr/src/sys/vm/vm_inherit.h */
#ifndef _SMP_SUPPORT
	log(LOG_INFO, "%s inher:%d wired_cnt:%d offset:%p\n", flagsstr,
	    current_entry->inheritance, current_entry->wired_count,
	    current_entry->offset);
#else
	log(LOG_INFO, "%s inher:%d wired_cnt:%d offset:0x%lx\n", flagsstr,
	    current_entry->inheritance, current_entry->wired_count,
	    (u_long) current_entry->offset);
#endif

	/* /sys/vm/vmobject.h */
	if (!(current_entry->eflags &
	      (MAP_ENTRY_IS_A_MAP | MAP_ENTRY_IS_SUB_MAP)) &&
	    current_entry->object.vm_object) {
/* 	  struct vm_object *vmo; */

	  vmflags2str(flagsstr, current_entry->object.vm_object->flags);
#ifndef _SMP_SUPPORT
	  log(LOG_INFO, " OBJECT:%p size:%d refcnt:%d flags:%s pager:%p off:%p [old: shadow:XXX off:XXX copy:XXX]\n",
	      current_entry->object.vm_object,
	      current_entry->object.vm_object->size,
	      current_entry->object.vm_object->ref_count,
	      flagsstr,
	      current_entry->object.vm_object->un_pager,
	      current_entry->object.vm_object->paging_offset);
#else
	  log(LOG_INFO, " OBJECT:%p size:%d refcnt:%d flags:%s pager:XXX off:0x%x [old: shadow:XXX off:XXX copy:XXX]\n",
	      current_entry->object.vm_object,
	      current_entry->object.vm_object->size,
	      current_entry->object.vm_object->ref_count,
	      flagsstr,
	      /* current_entry->object.vm_object->un_pager, */
	      (u_int) current_entry->object.vm_object->paging_offset);
#endif

	  /* Some changes occured between FreeBSD-2.1.0 and FreeBSD-2.2.5,
	     thus we need to comment some lines... */

/* 	      current_entry->object.vm_object->shadow, */
/* 	      current_entry->object.vm_object->shadow_offset, */
/* 	      current_entry->object.vm_object->copy); */

/*
	  if ((vmo = current_entry->object.vm_object->shadow))
	    for (; vmo; vmo = vmo->shadow) {
	      vmflags2str(flagsstr, vmo->flags);
	      log(LOG_INFO, " SHADOW:%p size:%d refcnt:%d flags:%s pager:%p off:%p shadow:%p off:%p copy:%p\n",
		  vmo, vmo->size, vmo->ref_count, flagsstr, vmo->pager,
		  vmo->paging_offset, vmo->shadow, vmo->shadow_offset,
		  vmo->copy);
	    }

	  if ((vmo = current_entry->object.vm_object->copy))
	    for (; vmo; vmo = vmo->copy) {
	      vmflags2str(flagsstr, vmo->flags);
	      log(LOG_INFO, " COPY:%p size:%d refcnt:%d flags:%s pager:%p off:%p shadow:%p off:%p copy:%p\n",
		  vmo, vmo->size, vmo->ref_count, flagsstr, vmo->pager,
		  vmo->paging_offset, vmo->shadow, vmo->shadow_offset,
		  vmo->copy);
	    }
*/
	}
	
      }

    }

    return 0;
    /* NOTREACHED */
    break;


    /************************************************************/

  case MPCOSGETOBJ:
    TRY("MPCOSGETOBJ")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MPCOSGETOBJ\n");
#endif

#ifdef MPC_OS
    *(vm_object_t *)data =  mpc_object_creation_fetch();
#endif

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSETMODEREAD:
    TRY("HSLSETMODEREAD")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLSETMODEREAD\n");
#endif

    return set_mode_read(minor(dev),
			 (set_mode_t *) data);
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLSETMODEWRITE:
    TRY("HSLSETMODEWRITE")

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "IOCTL HSLSETMODEWRITE\n");
#endif

    return set_mode_write(minor(dev),
			  (set_mode_t *) data);
    /* NOTREACHED */
    break;

    /************************************************************/

  case PCISETBUS:
    TRY("PCISETBUS")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL PCISETBUS\n");
#endif

#ifndef _SMP_SUPPORT
    pcibus_set_p = *(struct linker_set **) data;

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL PCISETBUS : _pcibus_set = %p\n", pcibus_set_p);
#endif
#endif

    return 0;

    /************************************************************/

  case DUMPCHANNELSTATE:
    TRY("DUMPCHANNELSTATE")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL DUMPCHANNELSTATE\n");
#endif

    log(LOG_DEBUG, "-------------- -------------------------\n");
    log(LOG_DEBUG, "-------------- DUMPING channel_state[][]\n");

    s = splhigh();

    for (i = 0; i < MAX_NODES - 1; i++)
      for (j = MIN_REALLOC_CHANNEL; j < CHANNEL_RANGE; j++) {
	if (channel_protocol[i][j] > HSL_PROTO_MDCP) {
	  log(LOG_ERR,
	      "channel_protocol[%d][%d] invalid\n",
	      i, j);
	  continue;
	}

	switch (channel_state[i][j]) {
	case CHAN_OPEN:
#ifndef _SMP_SUPPORT
	  log(LOG_DEBUG,
	      "channel[%d][%d] state OPENED - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      channel_classname[i][j],
	      classname2uid[channel_classname[i][j]]);
#else
	  log(LOG_DEBUG,
	      "channel[%d][%d] state OPENED - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) (classname2uid[channel_classname[i][j]]));
#endif
	  break;

	case CHAN_SHUTDOWN_0:
#ifndef _SMP_SUPPORT
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 0 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      channel_classname[i][j],
	      classname2uid[channel_classname[i][j]]);
#else
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 0 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);
#endif
	  break;

	case CHAN_SHUTDOWN_1:
#ifndef _SMP_SUPPORT
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 1 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      channel_classname[i][j],
	      classname2uid[channel_classname[i][j]]);
#else
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 1 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);
#endif
	  break;

	case CHAN_SHUTDOWN_2:
#ifndef _SMP_SUPPORT
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 2 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      channel_classname[i][j],
	      classname2uid[channel_classname[i][j]]);
#else
	  log(LOG_DEBUG,
	      "channel[%d][%d] state SHUTDOWN 2 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);
#endif
	  break;

	case CHAN_CLOSE:
	  break;

	default:
#ifndef _SMP_SUPPORT
	  log(LOG_WARNING,
	      "channel[%d][%d] invalid state - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      channel_classname[i][j],
	      classname2uid[channel_classname[i][j]]);
#else
	  log(LOG_WARNING,
	      "channel[%d][%d] invalid state - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);
#endif
	  break;
	}
      }

    splx(s);
    break;

    /************************************************************/

  case DUMPINTERNALTABLES:
    TRY("DUMPINTERNALTABLES")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL DUMPINTERNALTABLES\n");
#endif

    s = splhigh();

    log(LOG_DEBUG, "-------------- -------------------------\n");
    log(LOG_DEBUG, "-------------- DUMPING pending_receive[]\n");

    for (i = 0; i < PENDING_RECEIVE_SIZE; i++)
      if ((*pending_receive)[i].valid == VALID) {
	log(LOG_DEBUG,
	    "pending_receive[%d]: mi=0x%x chn=%d seq=%d stage=%d micp_seq=%d dst=%d\n",
	    i,
#ifdef _SMP_SUPPORT
	    (int)
#endif
	    (*pending_receive)[i].mi,
	    (*pending_receive)[i].channel,
	    (*pending_receive)[i].seq,
#ifdef _WITH_DDSCP
	    (*pending_receive)[i].stage,
	    (*pending_receive)[i].micp_seq,
#else
	    0,
	    0,
#endif
	    (*pending_receive)[i].sender);
      }

    log(LOG_DEBUG, "-------------- DUMPING pending_send[]\n");

    for (i = 0; i < PENDING_SEND_SIZE; i++)
      if (pending_send[i].valid != INVALID) {
	log(LOG_DEBUG,
	    "pending_send[%d]: valid=%d mi=0x%x chn=%d seq=%d dst=%d\n",
	    i,
	    pending_send[i].valid,
#ifdef _SMP_SUPPORT
	    (int)
#endif
	    pending_send[i].mi,
	    pending_send[i].channel,
	    pending_send[i].seq,
	    pending_send[i].dest);
      }

    log(LOG_DEBUG, "-------------- DUMPING received_receive[][]\n");

    for (i = 0; i < RECEIVED_RECEIVE_SIZE; i++)
      for (j = 0; j < MAX_NODES - 1; j++)
	if (recv_valid[i][j].general.valid == VALID) {
	  log(LOG_DEBUG,
	      "received_receive[%d][%d]: mi=0x%x chn=%d seq=%d dst=%d\n",
	      i, j,
#ifdef _SMP_SUPPORT
	      (int)
#endif
	      (*received_receive)[i][j].general.mi,
	      (*received_receive)[i][j].general.channel,
	      (*received_receive)[i][j].general.seq,
	      (*received_receive)[i][j].general.dest);
	}

    splx(s);
    break;

    /************************************************************/

  case HSLPROTECTWIRE:
    TRY("HSLPROTECTWIRE")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLPROTECTWIRE\n");
#endif

    return slrpv_prot_wire((vm_offset_t) ((struct _virtualRegion *) data)->start,
			   ((struct _virtualRegion *) data)->size,
			   NULL,
			   NULL,
			   p);
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLPROTECTDUMP:
    TRY("HSLPROTECTDUMP")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLPROTECTDUMP\n");
#endif

    slrpv_prot_dump();

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case SLRVGARBAGECOLLECT:
    TRY("SLRVGARBAGECOLLECT")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL SLRVGARBAGECOLLECT\n");
#endif

    slrpv_prot_garbage_collection();

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case MANAGEROPENCHANNEL:
    TRY("MANAGEROPENCHANNEL")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MANAGEROPENCHANNEL\n");
#endif

    switch (((manager_open_channel_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan0,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      res = slrpp_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan1,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
      /* NOTREACHED */
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan0,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      res = slrpv_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan1,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
      /* NOTREACHED */
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_init_com(((manager_open_channel_t *) data)->node,
			  ((manager_open_channel_t *) data)->chan0,
			  ((manager_open_channel_t *) data)->chan1,
			  ((manager_open_channel_t *) data)->classname,
			  p);
      if (res) return res;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[NODE2NODEINDEX(((manager_open_channel_t *) data)->node,
				      my_node)]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
      /* NOTREACHED */
      break;

    default:
      return EPROTONOSUPPORT;
      /* NOTREACHED */
      break;
    }

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case MANAGERSHUTDOWN1stSTEP:
    TRY("MANAGERSHUTDOWN1stSTEP")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MANAGERSHUTDOWN1stSTEP\n");
#endif

    switch (((manager_shutdown_1stStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
				    p);
      if (res) return res;

      res = slrpp_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
				    p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
				    p);
      if (res) return res;

      res = slrpv_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
				    p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_shutdown0_com(((manager_shutdown_1stStep_t *) data)->node,
			       ((manager_shutdown_1stStep_t *) data)->chan0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
			       p);
      return res;
      /* NOTREACHED */
      break;

    default:
      return EPROTONOSUPPORT;
      /* NOTREACHED */
      break;
    }

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case MANAGERSHUTDOWN2ndSTEP:
    TRY("MANAGERSHUTDOWN2ndSTEP")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MANAGERSHUTDOWN2ndSTEP\n");
#endif

    switch (((manager_shutdown_2ndStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
				    p);
      if (res) return res;

      res = slrpp_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
				    p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
				    p);
      if (res) return res;

      res = slrpv_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
				    p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_shutdown1_com(((manager_shutdown_2ndStep_t *) data)->node,
			       ((manager_shutdown_2ndStep_t *) data)->chan0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
			       ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
			       p);
      return res;
      /* NOTREACHED */
      break;

    default:
      return EPROTONOSUPPORT;
      /* NOTREACHED */
      break;
    }

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case MANAGERSHUTDOWN3rdSTEP:
    TRY("MANAGERSHUTDOWN3rdSTEP")

#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MANAGERSHUTDOWN3rdSTEP\n");
#endif

    switch (((manager_shutdown_3rdStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan0,
				p);
      if (res) return res;

      res = slrpp_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan1,
				p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan0,
				p);
      if (res) return res;

      res = slrpv_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan1,
				p);
      return res;
      /* NOTREACHED */
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_end_com(((manager_shutdown_3rdStep_t *) data)->node,
			 ((manager_shutdown_3rdStep_t *) data)->chan0,
			 p);
      if (res && res != EAGAIN && res != ENOTCONN && res != ENOENT)
	log(LOG_ERR, "error: mdcp_end_com() -> %d\n", res);

      return res;
      /* NOTREACHED */
      break;

    default:
      return EPROTONOSUPPORT;
      /* NOTREACHED */
      break;
    }

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case MANAGERSETAPPCLASSNAME:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL MANAGERSETAPPCLASSNAME\n");
#endif

    slrpp_set_appclassname(((manager_set_appclassname_t *) data)->cn,
			   ((manager_set_appclassname_t *) data)->uid);

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case LIBMPCREAD:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL LIBMPCREAD\n");
#endif

    if (((libmpc_read_t *) data)->channel < MIN_REALLOC_CHANNEL)
      return EPERM;

    /* This verification is not done inside mdcp_read(), and no such
       verification is done inside slrpv_recv() or slrpp_recv(), because
       it is much more efficient to do it once, instead of doing
       it each time a protocol layer is crossed. */
    if (slrpp_channel_check_rights(((libmpc_read_t *) data)->node,
				   ((libmpc_read_t *) data)->channel,
				   p) == FALSE)
      return EPERM;

    /* This verification can not be done inside mdcp_read() since mdcp_read()
       may be called when the channel is not in an opened state, in order
       to have it shutdown. */
    if (channel_state[NODE2NODEINDEX(((libmpc_read_t *) data)->node, my_node)]
                     [((libmpc_read_t *) data)->channel] != CHAN_OPEN) {
      ((libmpc_read_t *) data)->nbytes = 0;
      /* In order to emulate the standard behaviour of a read on an OPENED
	 file descriptor where EOF is reached, we MUST return with no error
	 here. A side-effect is that the user can not distinguish between an OPENED
	 channel where EOF is reached, and a CLOSED channel... */
      return /* should be ENOTCONN, but return 0 for the emulation */ 0;
    }

    if (channel_protocol[NODE2NODEINDEX(((libmpc_read_t *) data)->node, my_node)]
                        [((libmpc_read_t *) data)->channel] != HSL_PROTO_MDCP)
      return EPROTONOSUPPORT;

    res = mdcp_read(((libmpc_read_t *) data)->node,
		    ((libmpc_read_t *) data)->channel,
		    ((libmpc_read_t *) data)->buf,
		    ((libmpc_read_t *) data)->nbytes,
		    &((libmpc_read_t *) data)->nbytes,
		    p);

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case LIBMPCWRITE:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL LIBMPCWRITE\n");
#endif

    if (((libmpc_write_t *) data)->channel < MIN_REALLOC_CHANNEL)
      return EPERM;

    /* This verification is not done inside mdcp_write(), and no such
       verification is done inside slrpv_send() or slrpp_send(), because
       it is much more efficient to do it once, instead of doing
       it each time a protocol layer is crossed. */
    if (slrpp_channel_check_rights(((libmpc_write_t *) data)->node,
				   ((libmpc_write_t *) data)->channel,
				   p) == FALSE)
      return EPERM;

    /* This verification can not be done inside mdcp_write() since mdcp_write()
       may be called when the channel is not in an opened state, in order
       to have it shutdown. */
    if (channel_state[NODE2NODEINDEX(((libmpc_write_t *) data)->node, my_node)]
                     [((libmpc_write_t *) data)->channel] != CHAN_OPEN)
      return ENOTCONN;

    if (channel_protocol[NODE2NODEINDEX(((libmpc_write_t *) data)->node, my_node)]
                        [((libmpc_read_t *) data)->channel] != HSL_PROTO_MDCP)
      return EPROTONOSUPPORT;

    res = mdcp_write(((libmpc_write_t *) data)->node,
		    ((libmpc_write_t *) data)->channel,
		    (caddr_t) ((libmpc_write_t *) data)->buf,
		    ((libmpc_write_t *) data)->nbytes,
		    &((libmpc_write_t *) data)->nbytes,
		    p);

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case LIBMPCSELECT:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL LIBMPCSELECT\n");
#endif

    /* VERIFIER LES DROITS D'ACCES */

    res = hsl_select(p, (libmpc_select_t *) data,
		     &((libmpc_select_t *) data)->retval);

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLBENCHLATENCE1:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLBENCHLATENCE1\n");
#endif

    res = hsl_bench_latence1(minor(dev), (hsl_bench_latence_t *) data);

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLBENCHTHROUGHPUT1:
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "IOCTL HSLBENCHTHROUGHPUT1\n");
#endif

    res = hsl_bench_throughput1(minor(dev), (hsl_bench_throughput_t *) data);

    return res;
    /* NOTREACHED */
    break;

    /************************************************************/

  case HSLNOTICESELECT:

    do {
      res = tsleep(&selwait, PSOCK | PCATCH, "SeLeCt", NULL);
      if (!res)
	log(LOG_ERR, "GLOBAL SELECT WAKEUP\n");
    } while (!res);

    return 0;
    /* NOTREACHED */
    break;

    /************************************************************/

  case PCIDDCGETTABLESINFO:
#ifdef DEBUG_HSL
/*     log(LOG_DEBUG, "IOCTL PCIDDCGETTABLESINFO\n"); */
#endif

    get_tables_info(minor(dev), (tables_info_t *) data);

    return 0;
    /* NOTREACHED */
    break;
  }

  return 0;
}


/*
 ************************************************************
 * hsldriver_load(lkmtp, cmd) : handler for the load operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
hsldriver_load(lkmtp, cmd)
     struct lkm_table *lkmtp;
     int cmd;
{
  struct lkm_table lkmt;
  struct lkm_any lkma;
#ifndef _SMP_SUPPORT
  pcici_t tag;
#else
  pcicfgregs probe;
#endif
  int device;
  u_long val;
  int s;
  boolean_t notfound = TRUE;
  int current_minor = 0;

  TRY("hsldriver_load")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Load!\n");
#endif

  /* Accept to be loaded only if cmem driver already loaded */
  lkmt.private.lkm_any = &lkma;
  lkma.lkm_name = "cmemdriver_mod";
  if (!lkmexists(&lkmt)) {
    log(LOG_ERR,
	"Can't load hsldriver while cmemdriver not loaded.\n");
    return -1;
  }

  bzero(&driver_stats,
	sizeof driver_stats);

  init_events();

  ADD_EVENT(TRACE_INIT_DRIVER);

  slr_clear_config();
  put_init_SAP();

  /* Check for a PCIDDC or SmartHSL board */
  for (device = HSL_PCI_BUS;
       device < HSL_MAX_DEV;
       device++) {
    s = splhigh();
#ifndef _SMP_SUPPORT
    tag = getpcibus()->pb_tag(HSL_PCI_BUS,
			      device,
			      HSL_PCI_FUNC);
    val = getpcibus()->pb_read(tag,
			       PCI_ID_REG);
#else
    probe.bus  = HSL_PCI_BUS;
    probe.slot = device;
    probe.func = HSL_PCI_FUNC;
    val = pci_cfgread(&probe, PCI_ID_REG, 4);
#endif
    splx(s);

    if (val == DEVICEID_VENDORID_PCIDDCVALUE) {
      /* We found a board */
      notfound = FALSE;
      put_set_device(current_minor++,
#ifndef _SMP_SUPPORT
		     tag);
#else
                     probe);
#endif
      log(LOG_DEBUG,
	  "\nHSL board found : device %d\n", device);

#ifdef PCIDDC1stRun
      printf("\nVersion : PCIDDC 1st Run\n");
#endif
#ifdef PCIDDC2ndRun
      printf("\nVersion : PCIDDC 2nd Run\n");
#endif
    }
  }

  if (notfound == TRUE)
    log(LOG_DEBUG,
	"\nHSL board not found\n");

  printf("\nHSL driver loaded with success\n(c) Laboratoire LIP6 / Departement ASIM\n    Universite Pierre et Marie Curie (Paris 6)\n(feel free to contact fenyo@asim.lip6.fr for informations about this driver)\n\n");

  return 0;
}


/*
 ************************************************************
 * hsldriver_unload(lkmtp, cmd) : handler for the unload operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
hsldriver_unload(lkmtp, cmd)
     struct lkm_table *lkmtp;
     int cmd;
{
  TRY("hsldriver_unload")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "Unload!\n");
#endif

/*   if (open_count) */
/*     return -1; */

  close_slr();
  put_end_SAP();

  return 0;
}


/*
 ************************************************************
 * hsldriver_stat(lkmtp, cmd) : handler for the stat operation on the module.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload/stat).
 * 
 * return value : status of the operation.
 ************************************************************
 */

static int
hsldriver_stat(lkmtp, cmd)
        struct lkm_table *lkmtp;
        int cmd;
{
  return 0;
}


/*
 ************************************************************
 * hslinit(lkmtp, cmd) : entry point of the driver.
 * lkmtp : opaque structure.
 * cmd   : type of operation (load/unload).
 * ver   : version of the loading procedure applied to this driver by the system.
 * 
 * return value : status of the operation.
 ************************************************************
 */

int
hslinit(lkmtp, cmd, ver)
        struct lkm_table *lkmtp;
        int cmd;
        int ver;
{
  TRY("hslinit")

#ifndef _SMP_SUPPORT
#define _module hsldriver_module
  DISPATCH(hsldriver,
           lkmtp,
	   cmd,
	   ver,
	   hsldriver_load,
	   hsldriver_unload,
	   hsldriver_stat)
#else
  DISPATCH(hsldriver,
	   lkmtp,
	   cmd,
	   ver,
	   hsldriver_load,
	   hsldriver_unload,
	   hsldriver_stat)
#endif
}

