
#include <stdio.h>
#include <sys/fcntl.h>

#include <vm/vm.h>
#include <vm/vm_object.h>

#include "../modules/driver.h"

main()
{
  int fd;
  int res;
  vm_object_t obj;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  res = ioctl(fd, MPCOSGETOBJ, &obj);
  if (res < 0) {
    perror("ioctl MPCOSGETOBJ");
    exit(1);
  }

  printf("obj: %p\n", obj);

  close(fd);
  return 0;
}

