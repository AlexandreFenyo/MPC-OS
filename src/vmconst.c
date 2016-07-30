
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysent.h>
#include <sys/ioctl.h>

#define KERNEL
#include <sys/uio.h>
#undef KERNEL

#include <sys/systm.h>
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
#include <vm/lock.h>
#endif
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <machine/pmap.h>
#include <sys/errno.h>
#include <vm/vm_prot.h>
#include <machine/vmparam.h>

int
main(int ac, char **av)
{
  printf("VM_MIN_ADDR           %10p\n", (void *) VM_MIN_ADDRESS);
  printf("VM_MAXUSER_ADDRESS    %10p\n", (void *) VM_MAXUSER_ADDRESS);
  printf("USRSTACK              %10p\n", (void *) USRSTACK);
  printf("UPT_MIN_ADDRESS       %10p\n", (void *) UPT_MIN_ADDRESS);
  printf("UPT_MAX_ADDRESS       %10p\n", (void *) UPT_MAX_ADDRESS);
  printf("VM_MAX_ADDRESS        %10p\n", (void *) VM_MAX_ADDRESS);
  printf("VM_MIN_KERNEL_ADDRESS %10p\n", (void *) VM_MIN_KERNEL_ADDRESS);
  printf("KPT_MIN_ADDRESS       %10p\n", (void *) KPT_MIN_ADDRESS);
  printf("KPT_MAX_ADDRESS       %10p\n", (void *) KPT_MAX_ADDRESS);
  printf("VM_MAX_KERNEL_ADDRESS %10p\n", (void *) VM_MAX_KERNEL_ADDRESS);
  printf("VM_KMEM_SIZE          %10p\n", (void *) VM_KMEM_SIZE);

  return 0;
}


