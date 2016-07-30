
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#define SIZE (4096 * 16)
#define NB 100

struct timeval t_ref;

long
prtime(void)
{
  int res;
  struct timeval t;
  static struct timeval old_t;
  long diff;

  res = gettimeofday(&t, NULL);
  if (res < 0) {
    perror("gettimeofday");
    exit(1);
  }
  printf("time: %ld.%ld (%ld) - diff=%ld\n", t.tv_sec - t_ref.tv_sec, t.tv_usec,
	 ((long) t.tv_sec - t_ref.tv_sec) * 1000000 + ((long) t.tv_usec),
	  diff = ((long) t.tv_sec - old_t.tv_sec) * 1000000 +
	 ((long) t.tv_usec - old_t.tv_usec));

  old_t = t;
  return diff;
};


int
main(int ac, char **av)
{
  long diff;
  int fd, i;
  caddr_t addr;
  int res;

  if (ac < 2) {
    fprintf(stderr, "invalid number of arguments\n");
    exit(1);
  }

  gettimeofday(&t_ref, NULL);

  fd = open(av[1], O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  prtime();
  prtime();
  addr = mmap(NULL, SIZE, PROT_READ, 0, fd, 0);
  if (!addr) {
    perror("mmap");
    exit(1);
  }
  res = mlock(addr, SIZE);
  if (res < 0) {
    perror("mlock");
    exit(1);
  }
  prtime();
  addr = mmap(NULL, SIZE, PROT_READ, 0, fd, 0);
  prtime();

  for (i = 0; i < NB; i++) {
    addr = mmap(NULL, SIZE, PROT_READ, 0, fd, 0);
    if (!addr) {
      perror("mmap");
      exit(1);
    }

    /* printf("addr=%p\n", addr); */
  }
  diff = prtime();
  printf("average time : %ld\n", diff / NB);

  for (i = 0; i < 1000 * NB; i++) {
    getpid();
  }
  diff = prtime();
  printf("average time : %ld\n", diff / (1000 * NB));


  return 0;
}
