
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>

#include <kvm.h>
#include <nlist.h>

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"

int
main(ac, av)
     int ac;
     char **av;
{
  int i;
  int count;
  int fd;
  kvm_t *kd;
  struct linker_set *pcibus_set;

  struct nlist nl[] = {
    {"_pcibus_set", 0, 0, 0, 0},
    {NULL, 0, 0, 0, 0}
  };

  kd = kvm_open(NULL, NULL, NULL, O_RDONLY, av[0]);
  if (!kd) exit(1);

  count = kvm_nlist(kd, nl);

  if (count < 0) {
    printf("Kernel symbol table is unreadable\n");
    exit(1);
  }

  if (count != 0) {
    printf("no kernel symbol _pcibus_set\n");
    exit(1);
  }

  fd = open("/dev/hsl", O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  pcibus_set = (struct linker_set *) nl[0].n_value;

  i = ioctl(fd, PCISETBUS, &pcibus_set);
  if (i < 0) {
    perror("ioctl");
    exit(1);
  }

  kvm_close(kd);

  return 0;
}
