
#ifndef _USERACCESS_H_
#define _USERACCESS_H_

#include "../modules/driver.h"
#include "../modules/put.h"

#define KMEM_DEVICE "/dev/kmem"

extern int usrput_init __P((void));

extern int usrput_end __P((void));

extern opt_contig_mem_t *usrput_get_contig_mem __P((size_t));

extern int usrput_add_entry __P((lpe_entry_t *));

extern int usrput_get_node __P((void));

#endif
