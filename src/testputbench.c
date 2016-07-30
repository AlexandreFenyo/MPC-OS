
/* $Id: testputbench.c,v 1.3 2000/02/23 18:11:26 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>

#include "../MPC_OS/mpcshare.h"

#include "../src/useraccess.h"

#include "../modules/data.h"
#include "../modules/driver.h"

/* -F option :
root@isa-2-2-5# ../src/testputrecv
physical address = 0x4b1000
                                                        vvvvv-> 10000h
   ../src/testputbench -P1 -1 -i 12 -n 0 -a 0x4c1000 -F 65536 -o 8 -s 60000 -f 13
                                            ^^^^^^^^-> add 10000h to obtain 4c1000h

option -w : for local ping-pong (means 'w'ait for program whose MI value is...)

*/

#define FALSE   0
#define TRUE    1

#ifndef ever
#define ever ;;
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

/*typedef int boolean_t;*/

int fd;

boolean_t opt_first = TRUE;
boolean_t opt_check = FALSE;
boolean_t opt_debug = FALSE;
boolean_t opt_burst = FALSE;
int opt_offset = -1;
int opt_size = 0;
int opt_init_val = 0;
int opt_dist_node = -1;
int opt_page_size;
int opt_continue = 0;
int opt_prognum = 0;
int opt_fast = 0;
int opt_fstartval = 0;
int opt_foffset = 0;
int opt_waitforprg = -1;
u_long opt_dist_addr = 0;

void
usage(void)
{
  printf("usage:\ntestputbench [-b] [-d] [-1|-2] [-i initval] [-c] [-p pagesize] [-z]\n             [-P prognum] [-w prognum] [-f startval] [-F offval] -n node                     -a distaddr -o offset -s size\n");
  exit(0);
}

int
main(int ac, char **av)
{
  opt_contig_mem_t *contig;
  lpe_entry_t entry;
  int mi;
  int val;
  int ch;
  int i, jj;
  int res;
  int page_cnt = 0;
  int phase;
  u_char *PLSA_virt;
  boolean_t should_stop   = FALSE;
  boolean_t should_stop_2 = FALSE;

  opt_page_size = MIN(OPTIONAL_CONTIG_RAM_SIZE, 65535);

  while ((ch = getopt(ac, av, "d12o:s:ci:n:p:a:zbP:w:f:F:")) != EOF)
    switch (ch) {
    case '1':
      break;

    case '2':
      opt_first = FALSE;
      break;

    case 'd':
      opt_debug = TRUE;
      break;

    case 'o':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", &opt_offset);
      else opt_offset = atoi(optarg);
      break;

    case 's':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", &opt_size);
      else opt_size = atoi(optarg);
      break;

    case 'p':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", &opt_page_size);
      else opt_page_size = atoi(optarg);
      break;

    case 'a':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", (u_int *) &opt_dist_addr);
      else opt_dist_addr = atoi(optarg);
      break;

    case 'c':
      opt_check = TRUE;
      break;

    case 'b':
      opt_burst = TRUE;
      break;

    case 'i':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", (u_int *) &opt_init_val);
      else opt_init_val = atoi(optarg);
      break;

    case 'n':
      opt_dist_node = atoi(optarg);
      break;

    case 'z':
      opt_continue = TRUE;
      break;

    case 'P':
      opt_prognum = atoi(optarg);
      if (opt_prognum >= MAX_TESTPUTBENCH) {
	fprintf(stderr, "program number too high\n");
	exit(1);
      }
      break;

    case 'w':
      opt_waitforprg = atoi(optarg);
      if (opt_waitforprg >= MAX_TESTPUTBENCH) {
	fprintf(stderr, "program number too high\n");
	exit(1);
      }
      break;

    case 'f':
      opt_fast = TRUE;
      opt_fstartval = atoi(optarg);
      break;

    case 'F':
      opt_foffset = atoi(optarg);
      break;

    case '?':
    default:
      usage();
    }
  ac -= optind;
  av += optind;

  if (opt_fast && (opt_check | opt_debug | opt_burst)) {
    fprintf(stderr,
	    "incompatible options: -f & (-c | -d | -b)\n");
    usage();
  }

  if (opt_offset == -1 ||
      opt_size == 0 ||
      opt_dist_node == -1 ||
      opt_dist_addr == 0)
    usage();

  if (opt_page_size >= 65536) {
    fprintf(stderr,
	    "page size too big\n");
    exit(1);
  }

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  val = opt_prognum;
  res = ioctl(fd, HSLTESTPUTBENCH2, &val);
  if (res < 0) {
    perror("ioctl HSLTESTPUTBENCH2");
    exit(1);
  }
  mi = val;

  usrput_init();

  if (opt_waitforprg != -1) {
    char ch[16];
    printf("<PRESS RETURN>");
    fgets(ch, sizeof(ch) - 1, stdin);
  }

  contig = usrput_get_contig_mem(OPTIONAL_CONTIG_RAM_SIZE);
  contig->ptr  += opt_foffset;
  contig->phys += opt_foffset;

  if (contig == NULL) {
    fprintf(stderr, "Error in usrput library\n");
    exit(1);
  }

  if (opt_size + 2 * opt_offset > OPTIONAL_CONTIG_RAM_SIZE) {
    fprintf(stderr, "not enough memory\n");
    exit(1);
  }

  /* Set boundaries */
  memset(contig->ptr, opt_init_val, opt_offset);
  memset(contig->ptr + opt_offset + opt_size, opt_init_val, opt_offset);

  /* Initialize phase */
  phase = 0;

  /* Initialize data if no further initialization but further tests */
  if (opt_fast == TRUE)
    for (jj = 0; jj < opt_size; jj++)
      (jj + opt_offset)[contig->ptr] = (jj + opt_fstartval) % 256;

  if (opt_burst == TRUE)
    /* SENDING BURSTS */
    while (TRUE) {
      if (!(phase % 1000) || !((phase - 1) % 1000))
	fprintf(stdout,
		"phase %d\n",
		phase);

      /* Send data */
      if (opt_debug == TRUE)
	page_cnt = 0;

      /* Reset the window */
      if (opt_debug == TRUE)
	for (jj = 0; jj < opt_size; jj++)
	  (jj + opt_offset)[contig->ptr] = (jj + phase) % 256;

      for (PLSA_virt = contig->ptr + opt_offset;
	   PLSA_virt < contig->ptr + opt_offset + opt_size;
	   PLSA_virt += opt_page_size) {
	/* Prepare entry */
	entry.routing_part = opt_dist_node;
	entry.page_length =
	  MIN(opt_page_size,
	      contig->ptr + opt_offset + opt_size - PLSA_virt);
	entry.PLSA = PLSA_virt - contig->ptr + contig->phys;
	entry.PRSA = PLSA_virt - (u_long) contig->ptr + opt_dist_addr;
	entry.control =
	  (PLSA_virt + opt_page_size >= contig->ptr + opt_size) ?
	  NOR_MASK | LMP_MASK | LMI_ENABLE_MASK : 0;
	entry.control |= mi;

	/* Call put_add_entry() */
	if (opt_debug == TRUE)
	  fprintf(stderr,
		  "phase %d: send page %#x from [%#x,%#x[ to [%#x,%#x[\n",
		  phase,
		  page_cnt++,
		  (u_int) entry.PLSA,
		  (u_int) (entry.PLSA + opt_size),
		  (u_int) entry.PRSA,
		  (u_int) (entry.PRSA + opt_size));
	   restart1:
	res = ioctl(fd, HSLTESTPUTSEND1, &entry);
	if (res < 0) {
	  /* Probably LPE full */
#ifdef DEBUG_HSL
	  perror("ioctl HSLTESTPUTSEND1");
#endif
	  if (errno == EAGAIN) goto restart1;
	} else {
#ifdef DEBUG_HSL
	  fprintf(stderr,
		  "page sent with success\n");
#endif
	  phase++;
	}
      }
    }

  for (ever) {
    if (!(phase % 1000) || !((phase - 1) % 1000))
      fprintf(stderr,
	      "phase %d\n",
	      phase);

    phase++;

    /* Don't send data during the first phase of the second bench process */
    if (opt_first != FALSE || phase != 1) {
      if (opt_check == TRUE) {
	if (opt_debug == TRUE)
	  fprintf(stderr,
		  "reset boundaries and window\n");
	/* Reset the window */
	for (jj = 0; jj < opt_size; jj++)
	  (jj + opt_offset)[contig->ptr] = (jj + phase) % 256;
      }

      /* Send data */

      if (opt_debug == TRUE)
	page_cnt = 0;

      for (PLSA_virt = contig->ptr + opt_offset;
	   PLSA_virt < contig->ptr + opt_offset + opt_size;
	   PLSA_virt += opt_page_size) {
	/* Prepare entry */
	entry.routing_part = opt_dist_node;
	entry.page_length =
	  MIN(opt_page_size,
	      contig->ptr + opt_offset + opt_size - PLSA_virt);
	entry.PLSA = PLSA_virt - contig->ptr + contig->phys;
	entry.PRSA = PLSA_virt - (u_long) contig->ptr + opt_dist_addr;
	entry.control =
	  (PLSA_virt + opt_page_size >= contig->ptr + opt_size) ?
	  NOR_MASK | LMP_MASK | LMI_ENABLE_MASK : 0;
	entry.control |= mi;

	/* Call put_add_entry() */
	if (opt_debug == TRUE)
	  fprintf(stderr,
		  "phase %d: send page %#x from [%#x,%#x[ to [%#x,%#x[\n",
		  phase,
		  page_cnt++,
		  (u_int) entry.PLSA,
		  (u_int) (entry.PLSA + opt_size),
		  (u_int) entry.PRSA,
		  (u_int) (entry.PRSA + opt_size));
	   restart2:
	res = ioctl(fd, HSLTESTPUTSEND1, &entry);
	if (res < 0) {
	  perror("ioctl HSLTESTPUTSEND1");
	  if (errno == EAGAIN) goto restart2;
	  exit(1);
	}
      }

      /* Increment phase */
      phase++;
    }

    /* Wait for incoming data */
    if (opt_waitforprg == -1)
      val = opt_prognum;
    else
      val = opt_waitforprg;
    if (opt_debug == TRUE)
      fprintf(stderr,
	      "waiting for coming data (go to sleep)\n");
    res = ioctl(fd, HSLTESTPUTBENCH1, &val);
    if (res < 0) {
      perror("HSLTESTPUTBENCH1");
      exit(1);
    }

    /* Check data */
    if (opt_check == TRUE ||
	(opt_fast == TRUE && (!(phase % 1000) || !((phase - 1) % 1000)))) {
      if (opt_debug == TRUE || opt_fast == TRUE)
	fprintf(stderr,
		"checking window (phase %d): ",
		phase);

      /* Check that the window has not been corrupted */
      for (i = 0; i < opt_offset; i++) {
	if (contig->ptr[i] != opt_init_val) {
	  fprintf(stderr,
		  "p=%d bad window at %#x (phys=%#x) %#x, expected %#x\n",
		  phase,
		  (u_int) (contig->ptr + i),
		  (u_int) (contig->phys + i),
		  contig->ptr[i],
		  opt_init_val);
	  should_stop = TRUE;
	}

	if (contig->ptr[i + opt_offset + opt_size] != opt_init_val) {
	  fprintf(stderr,
		  "p=%d bad window at %#x (phys=%#x) %#x, expected %#x\n",
		  phase,
		  (u_int) (contig->ptr + i + opt_offset + opt_size),
		  (u_int) (contig->phys + i + opt_offset + opt_size),
		  contig->ptr[i + opt_offset + opt_size],
		  opt_init_val);
	  should_stop = TRUE;
	}
      }
      if ((opt_debug == TRUE || opt_fast == TRUE) && should_stop == FALSE)
	fprintf(stderr,
		"ok\n");

      /* Check that coming data are correct */
      if (opt_debug == TRUE || opt_fast == TRUE)
	fprintf(stderr,
		"checking data (phase %d): ",
		phase);
      if (opt_fast == FALSE) {
	for (jj = 0; jj < opt_size; jj++)
	  if ((jj + opt_offset)[contig->ptr] != (jj + phase) % 256) {
	    fprintf(stderr,
		    "p=%d bad data at %#x (phys=%#x) : %#x, expected %#x\n",
		    phase,
		    (u_int) (contig->ptr + jj + opt_offset),
		    (u_int) (contig->phys + jj + opt_offset),
		    (jj + opt_offset)[contig->ptr],
		    (jj + phase) % 256);
	    should_stop_2 = TRUE;
	  }
      } else {
	for (jj = 0; jj < opt_size; jj++)
	  if ((jj + opt_offset)[contig->ptr] != (jj + opt_fstartval) % 256) {
	    fprintf(stderr,
		    "p=%d bad data at %#x (phys=%#x) : %#x, expected %#x\n",
		    phase,
		    (u_int) (contig->ptr + jj + opt_offset),
		    (u_int) (contig->phys + jj + opt_offset),
		    (jj + opt_offset)[contig->ptr],
		    (jj + opt_fstartval) % 256);
	    should_stop_2 = TRUE;
	  }
      }
      if ((opt_debug == TRUE || opt_fast == TRUE) && should_stop_2 == FALSE)
	fprintf(stderr,
		"ok\n");

      if (opt_continue == FALSE && (should_stop == TRUE || should_stop_2 == TRUE))
	exit(1);
    }
  }

  usrput_end();

  close(fd);

  return 0;
}

