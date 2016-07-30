
/* $Id: bench_throughput.c,v 1.2 2000/02/07 18:52:11 alex Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>
#include <sys/types.h>

#include <stdlib.h>

#include "../MPC_OS/mpcshare.h"

#include "../src/useraccess.h"

#include "../modules/data.h"
#include "../modules/driver.h"

#define SIZE 1024
#if SIZE > OPTIONAL_CONTIG_RAM_SIZE
#error SIZE too big
#endif

#define FALSE   0
#define TRUE    1

int fd;

boolean_t opt_first = TRUE;
int opt_size = 0;
int opt_dist_node = -1;
int opt_count = 0;
u_long opt_dist_addr = 0;

void
usage(void)
{
  printf("usage:\nbench_throughput [-1|-2] -n node -a distaddr -s size -c count\n");
  exit(0);
}

int
main(int ac, char **av)
{
  int res;
  opt_contig_mem_t *contig;
  hsl_bench_throughput_t bench_throughput;
  int ch;

  while ((ch = getopt(ac, av, "12s:c:n:a:")) != EOF)
    switch (ch) {
    case '1':
      break;

    case '2':
      opt_first = FALSE;
      break;

    case 's':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", &opt_size);
      else opt_size = atoi(optarg);
      break;

    case 'a':
      if (!strncmp(optarg, "0x", 2))
	sscanf(optarg + 2, "%x", (u_int *) &opt_dist_addr);
      else opt_dist_addr = atoi(optarg);
      break;

    case 'n':
      opt_dist_node = atoi(optarg);
      break;

    case 'c':
      opt_count = atoi(optarg);
      break;

    case '?':
    default:
      usage();
    }
  ac -= optind;
  av += optind;

  if ((opt_first == TRUE) && (opt_size == 0 ||
			      opt_dist_node == -1 ||
			      opt_dist_addr == 0 ||
			      opt_count == 0))
    usage();
  if ((opt_first == FALSE) && (opt_size == 0 ||
			       opt_dist_node == -1 ||
			       opt_dist_addr == 0))
    usage();

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  usrput_init();

  contig = usrput_get_contig_mem(SIZE);

  if (contig == NULL) {
    fprintf(stderr, "Error in usrput library\n");
    exit(1);
  }

  bench_throughput.first           = opt_first;
  bench_throughput.size            = opt_size;
  bench_throughput.dist_node       = opt_dist_node;
  bench_throughput.count           = opt_count;
  bench_throughput.dist_addr       = opt_dist_addr;
  bench_throughput.local_addr      = (u_long) contig->phys;
  bench_throughput.local_addr_virt = contig->ptr;
  bench_throughput.wait_count      = 200000000;
  bench_throughput.result          = -1;

  res = ioctl(fd, HSLBENCHTHROUGHPUT1, &bench_throughput);
  if (res < 0) {
    perror("ioctl HSLBENCHTHROUGHPUT1");
    exit(1);
  }

  printf("RESULT: %d\n", bench_throughput.result);

  usrput_end();

  close(fd);

  return 0;
}

