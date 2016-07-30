
/* $Id: dumpPCIDDC.c,v 1.2 1998/12/18 17:57:41 alex Exp $ */

#include <stdio.h>
#include <sys/fcntl.h>

#include <sys/types.h>
#include <sys/mman.h>

#include <unistd.h>

#include <machine/param.h>

#include "/sys/pci/pcireg.h"

#include "../MPC_OS/mpcshare.h"
#include "../modules/driver.h"
#include "../modules/put.h"

int fd, fdmem;
u_char scan_device;
tables_info_t tables_info;
lpe_entry_t *lpe_p;
lmi_entry_t *lmi_p;
lrm_entry_t *lrm_p;

u_long
readpci(u_long reg)
{
  mpc_pci_conf_t conf;
  int res;

  conf.bus = HSL_PCI_BUS;
  conf.device = scan_device;
  conf.func = HSL_PCI_FUNC;
  conf.reg = reg;

  res = ioctl(fd, PCIREADCONF, &conf);
  if (res < 0) {
    perror("ioctl");
    exit(1);
  }

  return conf.data;
}

void
dump_lpe(void)
{
  int i;
  int current, new;

  current = (readpci(PCIDDC_LPE_CURRENT) - tables_info.lpe_phys) /
    sizeof(lpe_entry_t);
  new = (readpci(PCIDDC_LPE_NEW) - tables_info.lpe_phys) /
    sizeof(lpe_entry_t);

  printf("      +----------------------------------------------------------------------------------------------------+\n");
  for (i = 0; i < tables_info.lpe_size; i++) {
    if (i == current && i != new)
      printf("Cur-->");
    else
      if (i == new && i != current)
	printf("New-->");
      else
	if (i == new && i == current)
	  printf("C&N-->");
	else
	  printf("      ");

    printf("| %02x | mi=%06x | rp=%04x | lg=%04x | PRSA=%08x | PLSA=%08x | ",
	   i,
	   (u_int) (lpe_p[i].control & MI_MASK),
	   lpe_p[i].routing_part,
	   lpe_p[i].page_length,
	   (u_int) lpe_p[i].PRSA,
	   (u_int) lpe_p[i].PLSA);

    if (lpe_p[i].control & NOR_MASK)
      printf("NOR ");
    else
      printf("    ");

    if (lpe_p[i].control & LMP_MASK)
      printf("LMP ");
    else
      printf("    ");

    if (lpe_p[i].control & CM_MASK)
      printf("CM ");
    else
      printf("   ");

    if (lpe_p[i].control & LRM_ENABLE_MASK)
      printf("LRM ");
    else
      printf("    ");

    if (lpe_p[i].control & LMI_ENABLE_MASK)
      printf("LMI ");
    else
      printf("    ");

    if (lpe_p[i].control & SM_MASK)
      printf("SM ");
    else
      printf("   ");

    if (lpe_p[i].control & NOS_MASK)
      printf("NOS ");
    else
      printf("    ");

    if (lpe_p[i].control & RESERVED_MASK)
      printf("RES ");
    else
      printf("    ");

    printf("|\n");
  }
  printf("      +----------------------------------------------------------------------------------------------------+\n");
}

void
dump_lmi(void)
{
  int i;
  int current, new;

  current = (readpci(PCIDDC_LMI_CURRENT) - tables_info.lmi_phys) /
    sizeof(lmi_entry_t);
  new = (readpci(PCIDDC_LMI_NEW) - tables_info.lmi_phys) /
    sizeof(lmi_entry_t);

  printf("      +----------------------------------------------------------------------------------------------------+\n");
  for (i = 0; i < tables_info.lmi_size; i++) {
    if (i == current && i != new)
      printf("Cur-->");
    else
      if (i == new && i != current)
	printf("New-->");
      else
	if (i == new && i == current)
	  printf("C&N-->");
	else
	  printf("      ");

    printf("| %02x | mi=%06x | DATA1=%08x | DATA2=%08x | pn=%04x | r3=%02x | ",
	   i,
	   (u_int) lmi_p[i].control & MI_MASK,
	   (u_int) lmi_p[i].data1,
	   (u_int) lmi_p[i].data2,
	   lmi_p[i].packet_number,
	   lmi_p[i].r3_status);

    if (lmi_p[i].control & NOR_MASK)
      printf("NOR ");
    else
      printf("    ");

    if (lmi_p[i].control & LMP_MASK)
      printf("LMP ");
    else
      printf("    ");

    if (lmi_p[i].control & CM_MASK)
      printf("CM ");
    else
      printf("   ");

    if (lmi_p[i].control & LRM_ENABLE_MASK)
      printf("LRM ");
    else
      printf("    ");

    if (lmi_p[i].control & LMI_ENABLE_MASK)
      printf("LMI ");
    else
      printf("    ");

    if (lmi_p[i].control & SM_MASK)
      printf("SM ");
    else
      printf("   ");

    if (lmi_p[i].control & NOS_MASK)
      printf("NOS ");
    else
      printf("    ");

    if (lmi_p[i].control & RESERVED_MASK)
      printf("RES ");
    else
      printf("    ");

    printf("|\n");
  }
  printf("      +----------------------------------------------------------------------------------------------------+\n");
}

void
dump_lrm(void)
{
  int i;

  printf("      +---------------------------------+\n");
  for (i = 0; i < MIN(tables_info.lrm_size, 4); i++)
    printf("      | mi=%06x | EPN=%04x | RPN=%04x |\n",
	   i,
	   lrm_p[i].epn,
	   lrm_p[i].rpn);
  printf("      +---------------------------------+\n");
}

void
dump_status_register(void)
{
  u_long val;

  val = readpci(PCIDDC_STATUS);

  printf("Status Register :                                                \n");

  if (val & PCIDDC_STATUS_PAGE_TRANSMITTED)
    printf("[Page Transmitted]                                             \n");

  if (val & PCIDDC_STATUS_PAGE_RECEIVED)
    printf("[Page Received]                                                \n");

  if (val & PCIDDC_STATUS_PAGE_TRANSMITTED_4096)
    printf("[+4096 Pages Transmitted]                                      \n");

  if (val & PCIDDC_STATUS_PAGE_TRANSMITTED_256)
    printf("[+256 Pages Transmitted]                                       \n");

  if (val & PCIDDC_STATUS_PAGE_TRANSMITTED_16)
    printf("[+16 Pages Transmitted]                                        \n");

  if (val & PCIDDC_STATUS_PAGE_RECEIVED_4096)
    printf("[+4096 Pages Received]                                         \n");

  if (val & PCIDDC_STATUS_PAGE_RECEIVED_256)
    printf("[+256 Pages Received]                                          \n");

  if (val & PCIDDC_STATUS_PAGE_RECEIVED_16)
    printf("[+16 Pages Received]                                           \n");

  if (val & PCIDDC_STATUS_EEP_ERROR)
    printf("[EEP_ERROR]                                                    \n");

  if (val & PCIDDC_STATUS_CRC_HEADER_ERROR)
    printf("[CRC Header Error]                                             \n");

  if (val & PCIDDC_STATUS_CRC_DATA_ERROR)
    printf("[CRC Data Error]                                               \n");

  if (val & PCIDDC_STATUS_EP_ERROR)
    printf("[EP Error]                                                     \n");

  if (val & PCIDDC_STATUS_TIMEOUT_ERROR)
    printf("[Timeout Error]                                                \n");

  if (val & PCIDDC_STATUS_R3_STATUS_ERROR)
    printf("[R3 Status Error]                                              \n");

  if (val & PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR)
    printf("[Page Transmitted Overflow Error]                              \n");

  if (val & PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR)
    printf("[Page Received Overflow Error]                                 \n");

  if (val & PCIDDC_STATUS_SENT_PACKET_OF_ERROR)
    printf("[Sent Packet Overflow Error]                                   \n");

  if (val & PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR)
    printf("[Received Packet Overflow Error]                               \n");

  if (val & PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR)
    printf("[Illegal LRM Access Error]                                     \n");

  if (val & PCIDDC_STATUS_LMI_OF_ERROR)
    printf("[LMI Overflow Error]                                           \n");

  if (val & PCIDDC_STATUS_R3_STATUS_MASK)
    printf("[R3 Status : 0x%x]                                             \n",
	   (u_int) (val & PCIDDC_STATUS_R3_STATUS_MASK));
}

void
dump_status_and_command_register(void)
{
  u_long val;

  val = readpci(PCI_COMMAND_STATUS_REG);

  printf("Standard Status And Command Register :                           \n");

  if (val & PCI_STATUS_PARITY_DETECT)
    printf("[Status Parity Detect]                                         \n");

  if (val & PCI_STATUS_SPECIAL_ERROR)
    printf("[Status Special Error]                                         \n");

  if (val & PCI_STATUS_MASTER_ABORT)
    printf("[Status Master Abort]                                          \n");

  if (val & PCI_STATUS_MASTER_TARGET_ABORT)
    printf("[Status Master Target Abort]                                   \n");

  if (val & PCI_STATUS_TARGET_TARGET_ABORT)
    printf("[Status Target Target Abort]                                   \n");

  if (val & PCI_STATUS_DEVSEL_MASK)
    printf("[Status DEVSEL=0x%x]                                           \n",
	   (u_int) ((val & PCI_STATUS_DEVSEL_MASK)>>25));

  if (val & PCI_STATUS_PARITY_ERROR)
    printf("[Status Parity Error]                                          \n");

  if (val & PCI_STATUS_BACKTOBACK_OKAY)
    printf("[Status Fast Back-to-Back Capable]                             \n");

  if (val & 0x00400000UL)
    printf("[Status UDF Supported]                                         \n");

  if (val & 0x00200000UL)
    printf("[Status 66 MHz Capable]                                        \n");

  if (val & PCI_COMMAND_BACKTOBACK_ENABLE)
    printf("[Command Fast Back-to-Back Enable]                             \n");

  if (val & PCI_COMMAND_SERR_ENABLE)
    printf("[Command SERR Enable]                                          \n");

  if (val & PCI_COMMAND_STEPPING_ENABLE)
    printf("[Command Wait Cycle Control]                                   \n");

  if (val & PCI_COMMAND_PARITY_ENABLE)
    printf("[Command Parity Error Response]                                \n");

  if (val & PCI_COMMAND_PALETTE_ENABLE)
    printf("[Command VGA Palette Snoop]                                    \n");

  if (val & PCI_COMMAND_INVALIDATE_ENABLE)
    printf("[Command Memory Write and Invalidate Enable]                   \n");

  if (val & PCI_COMMAND_SPECIAL_ENABLE)
    printf("[Command Special Cycles]                                       \n");

  if (val & PCI_COMMAND_MASTER_ENABLE)
    printf("[Command Bus Master]                                           \n");

  if (val & PCI_COMMAND_MEM_ENABLE)
    printf("[Command Memory Space]                                         \n");

  if (val & PCI_COMMAND_IO_ENABLE)
    printf("[IO Space]                                                     \n");
}

int
main(int ac, char **av, char **ae)
{
  int res, device;
  int offset;
  mpc_pci_conf_t conf;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  fdmem = open("/dev/kmem", O_RDONLY);
  if (fdmem < 0) {
    perror("open");
    exit(1);
  }

  /* Find a board */

  for (device = HSL_PCI_BUS;
       device < HSL_MAX_DEV;
       device++) {
    conf.bus = HSL_PCI_BUS;
    conf.device = device;
    conf.func = HSL_PCI_FUNC;
    conf.reg = PCI_ID_REG;

    res = ioctl(fd, PCIREADCONF, &conf);
    if (res < 0) {
      perror("ioctl");
      exit(1);
    }

    if (conf.data == DEVICEID_VENDORID_PCIDDCVALUE)
      break; /* We found a board */
  }

  if (device == HSL_MAX_DEV) {
    fprintf(stderr, "No board found\n");
    exit(1);
  }

  scan_device = device;

  /* Map the PCI-DDC tables */

  res = ioctl(fd, PCIDDCGETTABLESINFO, &tables_info);
  if (res < 0) {
    perror("ioctl");
    exit(1);
  }

  offset = ((caddr_t) tables_info.lpe_virt) -
    (caddr_t) trunc_page((unsigned) tables_info.lpe_virt);
  lpe_p = (lpe_entry_t *) mmap(NULL,
			       tables_info.lpe_size *
			       sizeof(lpe_entry_t) +
			       offset,
			       PROT_READ,
			       MAP_SHARED,
			       fdmem,
			       (int) trunc_page((unsigned) tables_info.lpe_virt));
  if (lpe_p == (lpe_entry_t *) MAP_FAILED) {
    perror("mmap lpe");
    exit(1);
  }
  lpe_p = ((void *) lpe_p) + offset;

  offset = ((caddr_t) tables_info.lmi_virt) -
    (caddr_t) trunc_page((unsigned) tables_info.lmi_virt);
  lmi_p = (lmi_entry_t *) mmap(NULL,
			       tables_info.lmi_size *
			       sizeof(lmi_entry_t) +
			       offset,
			       PROT_READ,
			       MAP_SHARED,
			       fdmem,
			       (int) trunc_page((unsigned) tables_info.lmi_virt));
  if (lmi_p == (lmi_entry_t *) MAP_FAILED) {
    perror("mmap lmi");
    exit(1);
  }
  lmi_p = ((void *) lmi_p) + offset;

  offset = ((caddr_t) tables_info.lrm_virt) -
    (caddr_t) trunc_page((unsigned) tables_info.lrm_virt);
  lrm_p = (lrm_entry_t *) mmap(NULL,
			       tables_info.lrm_size *
			       sizeof(lrm_entry_t) +
			       offset,
			       PROT_READ,
			       MAP_SHARED,
			       fdmem,
			       (int) trunc_page((unsigned) tables_info.lrm_virt));
  if (lrm_p == (lrm_entry_t *) MAP_FAILED) {
    perror("mmap lrm");
    exit(1);
  }
  lrm_p = ((void *) lrm_p) + offset;

  printf("       LPE - virt=0x%08x - phys=0x%08x - size=%d entries\n",
	 (u_int) tables_info.lpe_virt,
	 (u_int) tables_info.lpe_phys,
	 tables_info.lpe_size);
  dump_lpe();
  printf("                                                                     \n");
  printf("       LMI - virt=0x%08x - phys=0x%08x - size=%d entries\n",
	 (u_int) tables_info.lmi_virt,
	 (u_int) tables_info.lmi_phys,
	 tables_info.lmi_size);

  dump_lmi();
  printf("                                                                     \n");
  printf("       LRM - virt=0x%08x - phys=0x%08x - size=%d entries\n",
	 (u_int) tables_info.lrm_virt,
	 (u_int) tables_info.lrm_phys,
	 tables_info.lrm_size);
  dump_lrm();
  printf("                                                                     \n");
  dump_status_register();
  printf("                                                                     \n");
  dump_status_and_command_register();
  printf("                                                                     \n");
  printf("                                                                     \n");
  printf("                                                                     \n");
  printf("                                                                     \n");
  printf("                                                                     \n");

  close(fdmem);
  close(fd);
  exit(0);
}


