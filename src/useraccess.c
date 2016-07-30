
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"

#include "useraccess.h"

extern int errno;

static int dev = -1;

static int
opendev()
{
  if (dev < 0) {
    dev = open("/dev/hsl",
	       O_RDWR);
    if (dev < 0)
      return -1;
  }  
  return 0;
}

int
usrput_init()
{
  return 0;
}

int
usrput_end()
{
  return 0;
}

int
usrput_get_node()
{
  int node;
  int res;
  
  res = opendev();
  if (res < 0)
    return -1;

  res = ioctl(dev,
	      HSLGETNODE,
	      &node);

  if (res < 0)
    return -1;

  return node;
}

opt_contig_mem_t *
usrput_get_contig_mem(size)
                       size_t size;
{
  int fd;
  int res;
  static opt_contig_mem_t opt;
  caddr_t mmap_addr;

  res = opendev();
  if (res < 0)
    return NULL;

  res = ioctl(dev,
	      HSLGETOPTCONTIGMEM,
	      &opt);

  if (res < 0 ||
      !opt.ptr)
    return NULL;

  fd = open(KMEM_DEVICE,
	    O_RDWR);
  if (fd < 0)
    return NULL;

  mmap_addr = mmap(NULL,
		   opt.size,
		   PROT_READ|PROT_WRITE,
		   MAP_SHARED,
		   fd,
		   (int) opt.ptr);
  close(fd);

  if (mmap_addr == (caddr_t) -1)
    return NULL;    

  opt.ptr = mmap_addr;

  return &opt;
}

int
usrput_add_entry(entry)
lpe_entry_t *entry;
{
  int res;

  res = opendev();
  if (res < 0)
    return -1;

  res = ioctl(dev,
	      HSLADDLPE,
	      entry);

  return res;
}


