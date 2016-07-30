
/* $Id: mpcview_main.c,v 1.2 1999/01/09 18:58:48 alex Exp $ */

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>

#include <stdio.h>
#include <time.h>

#include <sys/time.h>
#include <sys/signal.h>
#include <sys/fcntl.h>

#ifndef _SMP_SUPPORT
#include "/sys/pci/pcireg.h"
#endif

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"

#include "forms.h"
#include "mpcview.h"

extern int errno;

int fd;
u_char scan_device;

#define NPOINTS 100
#define YMAX 80
float scan_x[NPOINTS];
float scan_y[NPOINTS];

FD_mpcview *FD;

int button_0 = 0;
int button_1 = 0;
int button_2 = 0;
int button_3 = 0;
int button_4 = 0;
int button_5 = 0;
int button_6 = 0;
int button_7 = 0;

int button_err_0 = 0;
int button_err_1 = 0;
int button_err_2 = 0;
int button_err_3 = 0;
int button_err_4 = 0;
int button_err_5 = 0;
int button_err_6 = 0;
int button_err_7 = 0;
int button_err_8 = 0;
int button_err_9 = 0;
int button_err_10 = 0;
int button_err_11 = 0;

int button_other_0 = 0;
int button_other_1 = 0;
int button_other_2 = 0;
int button_other_3 = 0;

int slider_0 = 0;
int slider_1 = 0;

int nevents = 0;

/* animation */
unsigned int w_ret, h_ret;
int xhot_ret, yhot_ret;
Pixmap b_ret;
int defil_cpt_sens;
int defil_sens;
int notice2_popedup = 0;
int defil_xoffset;
int defil_yoffset;
int defil_interval;
int defil_interval_first;
int defil_ligne[1024];
int defil_haut;
Display *defil_disp;
Window defil_dest;
GC gc;
XGCValues gc_values;
Pixmap pix_defil;
int pix_w, pix_h;

void
decal_points(float f)
{
  int i;

  for (i = 0; i < NPOINTS - 1; i++) scan_y[i] = scan_y[i+1];
  scan_y[NPOINTS - 1] = f;
  fl_set_xyplot_data(FD->scan,
		     scan_x,
		     scan_y,
		     NPOINTS,
		     "THROUGHPUT",
		     "seconds", "MB/s");
}

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
writepci(u_long reg, u_long val)
{
  mpc_pci_conf_t conf;
  int res;

  conf.bus    = HSL_PCI_BUS;
  conf.device = scan_device;
  conf.func   = HSL_PCI_FUNC;
  conf.reg    = reg;
  conf.data   = val;

  res = ioctl(fd, PCIWRITECONF, &conf);
  if (res < 0) {
    perror("ioctl");
    exit(1);
  }
}

void
toglepci(u_long reg, u_long val)
{
  u_long content;

  content = readpci(reg);
  if (content & val)
    writepci(reg, content & ~val);
  else
    writepci(reg, content | val);
}

void
cb_read(void)
{
  u_long reg, regcmd;
  int res;

  /*
    val = value;
    fl_set_slider_value(FD->average, MIN(val, YMAX));
    decal_points((float) val);
  */

  reg    = readpci(PCIDDC_STATUS);
  regcmd = readpci(PCIDDC_COMMAND);

  /************************************************************/

  if (reg & PCIDDC_STATUS_EEP_ERROR && !button_err_0)
    fl_set_button(FD->button_err_0,
		  button_err_0 = 1);

  if (reg & PCIDDC_STATUS_CRC_HEADER_ERROR && !button_err_1)
    fl_set_button(FD->button_err_1,
		  button_err_1 = 1);

  if (reg & PCIDDC_STATUS_CRC_DATA_ERROR && !button_err_2)
    fl_set_button(FD->button_err_2,
		  button_err_2 = 1);

  if (reg & PCIDDC_STATUS_EP_ERROR && !button_err_3)
    fl_set_button(FD->button_err_3,
		  button_err_3 = 1);

  if (reg & PCIDDC_STATUS_TIMEOUT_ERROR && !button_err_4)
    fl_set_button(FD->button_err_4,
		  button_err_4 = 1);

  if (reg & PCIDDC_STATUS_R3_STATUS_ERROR && !button_err_5)
    fl_set_button(FD->button_err_5,
		  button_err_5 = 1);

  if (reg & PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR && !button_err_6)
    fl_set_button(FD->button_err_6,
		  button_err_6 = 1);

  if (reg & PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR && !button_err_7)
    fl_set_button(FD->button_err_7,
		  button_err_7 = 1);

  if (reg & PCIDDC_STATUS_SENT_PACKET_OF_ERROR && !button_err_8)
    fl_set_button(FD->button_err_8,
		  button_err_8 = 1);

  if (reg & PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR && !button_err_9)
    fl_set_button(FD->button_err_9,
		  button_err_9 = 1);

  if (reg & PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR && !button_err_10)
    fl_set_button(FD->button_err_10,
		  button_err_10 = 1);

  if (reg & PCIDDC_STATUS_LMI_OF_ERROR && !button_err_11)
    fl_set_button(FD->button_err_11,
		  button_err_11 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L0 && !button_0)
    fl_set_button(FD->button_0,
		  button_0 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L1 && !button_1)
    fl_set_button(FD->button_1,
		  button_1 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L2 && !button_2)
    fl_set_button(FD->button_2,
		  button_2 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L3 && !button_3)
    fl_set_button(FD->button_3,
		  button_3 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L4 && !button_4)
    fl_set_button(FD->button_4,
		  button_4 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L5 && !button_5)
    fl_set_button(FD->button_5,
		  button_5 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L6 && !button_6)
    fl_set_button(FD->button_6,
		  button_6 = 1);

  if (reg & PCIDDC_STATUS_IRQ_L7 && !button_7)
    fl_set_button(FD->button_7,
		  button_7 = 1);

  /************************************************************/

  if (!(reg & PCIDDC_STATUS_EEP_ERROR) && button_err_0)
    fl_set_button(FD->button_err_0,
		  button_err_0 = 0);

  if (!(reg & PCIDDC_STATUS_CRC_HEADER_ERROR) && button_err_1)
    fl_set_button(FD->button_err_1,
		  button_err_1 = 0);

  if (!(reg & PCIDDC_STATUS_CRC_DATA_ERROR) && button_err_2)
    fl_set_button(FD->button_err_2,
		  button_err_2 = 0);

  if (!(reg & PCIDDC_STATUS_EP_ERROR) && button_err_3)
    fl_set_button(FD->button_err_3,
		  button_err_3 = 0);

  if (!(reg & PCIDDC_STATUS_TIMEOUT_ERROR) && button_err_4)
    fl_set_button(FD->button_err_4,
		  button_err_4 = 0);

  if (!(reg & PCIDDC_STATUS_R3_STATUS_ERROR) && button_err_5)
    fl_set_button(FD->button_err_5,
		  button_err_5 = 0);

  if (!(reg & PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR) && button_err_6)
    fl_set_button(FD->button_err_6,
		  button_err_6 = 0);

  if (!(reg & PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR) && button_err_7)
    fl_set_button(FD->button_err_7,
		  button_err_7 = 0);

  if (!(reg & PCIDDC_STATUS_SENT_PACKET_OF_ERROR) && button_err_8)
    fl_set_button(FD->button_err_8,
		  button_err_8 = 0);

  if (!(reg & PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR) && button_err_9)
    fl_set_button(FD->button_err_9,
		  button_err_9 = 0);

  if (!(reg & PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR) && button_err_10)
    fl_set_button(FD->button_err_10,
		  button_err_10 = 0);

  if (!(reg & PCIDDC_STATUS_LMI_OF_ERROR) && button_err_11)
    fl_set_button(FD->button_err_11,
		  button_err_11 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L0) && button_0)
    fl_set_button(FD->button_0,
		  button_0 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L1) && button_1)
    fl_set_button(FD->button_1,
		  button_1 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L2) && button_2)
    fl_set_button(FD->button_2,
		  button_2 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L3) && button_3)
    fl_set_button(FD->button_3,
		  button_3 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L4) && button_4)
    fl_set_button(FD->button_4,
		  button_4 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L5) && button_5)
    fl_set_button(FD->button_5,
		  button_5 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L6) && button_6)
    fl_set_button(FD->button_6,
		  button_6 = 0);

  if (!(reg & PCIDDC_STATUS_IRQ_L7) && button_7)
    fl_set_button(FD->button_7,
		  button_7 = 0);

  /************************************************************/

  if (regcmd & PCIDDC_Tx_INIT && !button_other_0)
    fl_set_button(FD->button_other_0,
		  button_other_0 = 1);

  if (regcmd & PCIDDC_R3_INIT && !button_other_1)
    fl_set_button(FD->button_other_1,
		  button_other_1 = 1);

  if (regcmd & PCIDDC_Rx_INIT && !button_other_2)
    fl_set_button(FD->button_other_2,
		  button_other_2 = 1);

  if (regcmd & PCIDDC_LOOPBACK && !button_other_3)
    fl_set_button(FD->button_other_3,
		  button_other_3 = 1);

  /************************************************************/

  if (!(regcmd & PCIDDC_Tx_INIT) && button_other_0)
    fl_set_button(FD->button_other_0,
		  button_other_0 = 0);

  if (!(regcmd & PCIDDC_R3_INIT) && button_other_1)
    fl_set_button(FD->button_other_1,
		  button_other_1 = 0);

  if (!(regcmd & PCIDDC_Rx_INIT) && button_other_2)
    fl_set_button(FD->button_other_2,
		  button_other_2 = 0);

  if (!(regcmd & PCIDDC_LOOPBACK) && button_other_3)
    fl_set_button(FD->button_other_3,
		  button_other_3 = 0);

  /************************************************************/

  res = ((readpci(PCIDDC_COMMAND) &
	  PCIDDC_TXRX_BALANCE_MASK)>>PCIDDC_TXRX_BALANCE_START) - 8;
  if (res != slider_0)
    fl_set_slider_value(FD->tx_rx,
			slider_0 = res);

  res = (readpci(PCIDDC_COMMAND) &
	 PCIDDC_MAX_PACKET_LENGTH_MASK)>>PCIDDC_MAX_PACKET_LENGTH_START;
  if (res != slider_1)
    fl_set_slider_value(FD->pack_len,
			slider_1 = res);
}

void
cb_tx_rx(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  u_long val;

  val = readpci(PCIDDC_COMMAND) & ~PCIDDC_TXRX_BALANCE_MASK;

  writepci(PCIDDC_COMMAND,
	   val |
	   ((u_long) (8 + (slider_0 = fl_get_slider_value(obj))))<<
	   PCIDDC_TXRX_BALANCE_START);
}

void
cb_pack_len(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  u_long val;

  val = readpci(PCIDDC_COMMAND) & ~PCIDDC_MAX_PACKET_LENGTH_MASK;

  writepci(PCIDDC_COMMAND,
	   val |
	   ((u_long) (slider_1 = fl_get_slider_value(obj)))<<
	   PCIDDC_MAX_PACKET_LENGTH_START);
}

void
cb_button_0(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_0, button_0);
}

void
cb_button_1(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_1, button_1);
}

void
cb_button_2(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_2, button_2);
}

void
cb_button_3(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_3, button_3);
}

void
cb_button_4(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_4, button_4);
}

void
cb_button_5(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_5, button_5);
}

void
cb_button_6(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_6, button_6);
}

void
cb_button_7(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  fl_set_button(FD->button_7, button_7);
}

/************************************************************/

void
cb_button_err_0(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_EEP_ERROR);
  fl_set_button(FD->button_err_0, button_err_0);
}

void
cb_button_err_1(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_CRC_HEADER_ERROR);
  fl_set_button(FD->button_err_1, button_err_1);
}

void
cb_button_err_2(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_CRC_DATA_ERROR);
  fl_set_button(FD->button_err_2, button_err_2);
}

void
cb_button_err_3(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_EP_ERROR);
  fl_set_button(FD->button_err_3, button_err_3);
}

void
cb_button_err_4(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_TIMEOUT_ERROR);
  fl_set_button(FD->button_err_4, button_err_4);
}

void
cb_button_err_5(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_R3_STATUS_ERROR);
  fl_set_button(FD->button_err_5, button_err_5);
}

void
cb_button_err_6(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR);
  fl_set_button(FD->button_err_6, button_err_6);
}

void
cb_button_err_7(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR);
  fl_set_button(FD->button_err_7, button_err_7);
}

void
cb_button_err_8(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_SENT_PACKET_OF_ERROR);
  fl_set_button(FD->button_err_8, button_err_8);
}

void
cb_button_err_9(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR);
  fl_set_button(FD->button_err_9, button_err_9);
}

void
cb_button_err_10(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR);
  fl_set_button(FD->button_err_10, button_err_10);
}

void
cb_button_err_11(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  writepci(PCIDDC_STATUS,
	    PCIDDC_STATUS_LMI_OF_ERROR);
  fl_set_button(FD->button_err_11, button_err_11);
}

/************************************************************/

void
cb_button_other_0(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  toglepci(PCIDDC_COMMAND,
	    PCIDDC_Tx_INIT);
  /*fl_set_button(FD->button_other_0, button_other_0);*/
}

void
cb_button_other_1(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  toglepci(PCIDDC_COMMAND,
	    PCIDDC_R3_INIT);
  /*fl_set_button(FD->button_other_1, button_other_1);*/
}

void
cb_button_other_2(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  toglepci(PCIDDC_COMMAND,
	    PCIDDC_Rx_INIT);
  /*fl_set_button(FD->button_other_2, button_other_2);*/
}

void
cb_button_other_3(obj, arg)
     FL_OBJECT *obj;
     void *arg;
{
  toglepci(PCIDDC_COMMAND,
	   PCIDDC_LOOPBACK);
  /*fl_set_button(FD->button_other_3, button_other_3);*/
}

/************************************************************/

int
post_handler(ob, event, mx, my, key, xev)
             FL_OBJECT *ob;
             int event;
             FL_Coord mx;
             FL_Coord my;
             int key;
             void *xev;
{
  char *bulle;
  time_t tloc;

  if (ob == FD->button_0)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_1)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_2)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_3)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_4)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_5)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_6)
    bulle = "Checked when 1Gbit/s link is calibrated";
  else
  if (ob == FD->button_7)
    bulle = "Checked when 1Gbit/s link is calibrated";

  else
  if (ob == FD->clock) {
    time(&tloc);
    bulle = ctime(&tloc);
    bulle[strlen(bulle) - 1] = 0;
  }
  else bulle = NULL;

  if (bulle && (event == FL_ENTER)) {
    fl_show_oneliner(bulle, ob->form->x + ob->x,
		     ob->form->y + ob->y + ob->h + 5);
  }

  if (bulle && (event == FL_LEAVE))
    fl_hide_oneliner();

  return 0;
}

/* DEFILEMENT */
void
defil_sens_bas(i)
     int i;
{
  XCopyArea(defil_disp, pix_defil, defil_dest, gc, 0, i, pix_w, 1,
	    defil_xoffset, defil_yoffset + 1 + defil_ligne[i]);
  defil_ligne[i]++;
  XFlush(defil_disp);
}

int
defil_mvt1()
{
  int i;
  int res=0;

  for (i=0; i < (defil_haut - 1); i++)
    if ((defil_ligne[i] + 1) < defil_ligne[i+1]) {
      defil_sens_bas(i);
      res = 1;
    }

  if (defil_ligne[defil_haut-1] < (2*defil_haut - 1)) {
    defil_sens_bas(defil_haut - 1);
    res = 1;
  }

  return res;
}

void
defil_sens_haut(i)
     int i;
{
  XCopyArea(defil_disp, pix_defil, defil_dest, gc, 0, i, pix_w, 1,
	    defil_xoffset, defil_yoffset + defil_ligne[i] - 1);
  defil_ligne[i]--;
  XFlush(defil_disp);
}

int
defil_mvt2()
{
  int i;
  int res=0;

  for (i = defil_haut - 1; i > 0; i--)
    if ((defil_ligne[i] - 1) > defil_ligne[i-1]) {
      defil_sens_haut(i);
      res = 1;
    }
  if (defil_ligne[0] > 0) {
    defil_sens_haut(0);
    res = 1;
  }

  return res;
}

void
sig_alarm(int signum, void *param)
{
  int res;
  static int cpt;

  if (!(cpt++%10)) cb_read();

  if (!defil_sens) res = defil_mvt1();
  else res = defil_mvt2();

  if (!res) {
    defil_sens = ~defil_sens;
    defil_cpt_sens++;
  }
}

int
main(int argc, char *argv[])
{
  FD_mpcview *fd_mpcview;
  int i, res;
  struct itimerval it;
  XGCValues gc_values;
  int device;
  mpc_pci_conf_t conf;

  fd = open("/dev/hsl", O_RDWR);
  if (fd < 0) {
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

  fl_initialize(&argc, argv, 0, 0, 0);
  FD = fd_mpcview = create_form_mpcview();

  fl_set_object_callback(fd_mpcview->tx_rx,
			 (FL_CALLBACKPTR) cb_tx_rx,
			 NULL);
  fl_set_object_callback(fd_mpcview->pack_len,
			 (FL_CALLBACKPTR) cb_pack_len,
			 NULL);

  fl_set_object_callback(fd_mpcview->button_0, cb_button_0,
			 (long) &button_0);
  fl_set_object_callback(fd_mpcview->button_1, cb_button_1,
			 (long) &button_1);
  fl_set_object_callback(fd_mpcview->button_2, cb_button_2,
			 (long) &button_2);
  fl_set_object_callback(fd_mpcview->button_3, cb_button_3,
			 (long) &button_3);
  fl_set_object_callback(fd_mpcview->button_4, cb_button_4,
			 (long) &button_4);
  fl_set_object_callback(fd_mpcview->button_5, cb_button_5,
			 (long) &button_5);
  fl_set_object_callback(fd_mpcview->button_6, cb_button_6,
			 (long) &button_6);
  fl_set_object_callback(fd_mpcview->button_7, cb_button_7,
			 (long) &button_7);

  fl_set_object_callback(fd_mpcview->button_err_0, cb_button_err_0,
			 (long) &button_err_0);
  fl_set_object_callback(fd_mpcview->button_err_1, cb_button_err_1,
			 (long) &button_err_1);
  fl_set_object_callback(fd_mpcview->button_err_2, cb_button_err_2,
			 (long) &button_err_2);
  fl_set_object_callback(fd_mpcview->button_err_3, cb_button_err_3,
			 (long) &button_err_3);
  fl_set_object_callback(fd_mpcview->button_err_4, cb_button_err_4,
			 (long) &button_err_4);
  fl_set_object_callback(fd_mpcview->button_err_5, cb_button_err_5,
			 (long) &button_err_5);
  fl_set_object_callback(fd_mpcview->button_err_6, cb_button_err_6,
			 (long) &button_err_6);
  fl_set_object_callback(fd_mpcview->button_err_7, cb_button_err_7,
			 (long) &button_err_7);
  fl_set_object_callback(fd_mpcview->button_err_8, cb_button_err_8,
			 (long) &button_err_8);
  fl_set_object_callback(fd_mpcview->button_err_9, cb_button_err_9,
			 (long) &button_err_9);
  fl_set_object_callback(fd_mpcview->button_err_10, cb_button_err_10,
			 (long) &button_err_10);
  fl_set_object_callback(fd_mpcview->button_err_11, cb_button_err_11,
			 (long) &button_err_11);

  fl_set_object_callback(fd_mpcview->button_other_0, cb_button_other_0,
			 (long) &button_other_0);
  fl_set_object_callback(fd_mpcview->button_other_1, cb_button_other_1,
			 (long) &button_other_1);
  fl_set_object_callback(fd_mpcview->button_other_2, cb_button_other_2,
			 (long) &button_other_2);
  fl_set_object_callback(fd_mpcview->button_other_3, cb_button_other_3,
			 (long) &button_other_3);

  /* fill-in form initialization code */

  fl_set_object_bw(fd_mpcview->all_objects, -2);

  fl_set_menu(fd_mpcview->menufile, "Exit");

  fl_set_menu_item_shortcut(fd_mpcview->menufile, 1, "Cc#C#c");
  fl_set_menu_item_shortcut(fd_mpcview->menufile, 2, "Ee#E#e");

  fl_set_browser_fontsize(fd_mpcview->browser, 12);
  fl_set_browser_fontstyle(fd_mpcview->browser, FL_FIXED_STYLE);

  fl_set_slider_bounds(fd_mpcview->average, YMAX, 0);
  fl_set_slider_value(fd_mpcview->average, 0);

  fl_set_slider_bounds(fd_mpcview->tx_rx, -8, 7);
  slider_0 = ((readpci(PCIDDC_COMMAND) &
	      PCIDDC_TXRX_BALANCE_MASK)>>PCIDDC_TXRX_BALANCE_START) - 8;
  fl_set_slider_value(fd_mpcview->tx_rx, slider_0);
  fl_set_slider_step(fd_mpcview->tx_rx, 1);
  fl_set_slider_precision(fd_mpcview->tx_rx, 0);

  fl_set_slider_bounds(fd_mpcview->pack_len, 0, 4095);
  slider_1 = (readpci(PCIDDC_COMMAND) &
	      PCIDDC_MAX_PACKET_LENGTH_MASK)>>PCIDDC_MAX_PACKET_LENGTH_START;
  fl_set_slider_value(fd_mpcview->pack_len, slider_1);
  fl_set_slider_step(fd_mpcview->pack_len, 1);
  fl_set_slider_precision(fd_mpcview->pack_len, 0);

  fl_set_pixmap_file(fd_mpcview->logo, "logo.xpm");

  /* bulles d'info */
  fl_set_object_posthandler(fd_mpcview->button_0, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_1, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_2, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_3, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_4, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_5, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_6, post_handler);
  fl_set_object_posthandler(fd_mpcview->button_7, post_handler);

  /* CHART */

  FD->scan->use_pixmap = 1;

  fl_set_chart_bounds(FD->scanchart, 0, YMAX);
  fl_set_chart_maxnumb(FD->scanchart,NPOINTS);
  fl_set_chart_autosize(FD->scanchart, 0);

  /* show the first form */
  fl_show_form(fd_mpcview->mpcview,FL_PLACE_CENTER,FL_FULLBORDER,"mpcview");

  /* animation */
  it.it_interval.tv_sec  = it.it_value.tv_sec  = 0;
  it.it_interval.tv_usec = it.it_value.tv_usec = 20000;
  setitimer(ITIMER_REAL, &it, NULL);

  fl_add_signal_callback(SIGALRM, sig_alarm, NULL);

  gc_values.foreground = BlackPixel(fl_display, DefaultScreen(fl_display));
  gc_values.background = fl_get_pixel(FL_GRAY80);

  gc_values.graphics_exposures = False;
  gc = XCreateGC(fl_display, FL_ObjWin(FD->browser),
		 GCGraphicsExposures|GCForeground|GCBackground, &gc_values);

  if (XReadBitmapFile(fl_display, FL_ObjWin(FD->browser), "defil.bitmap",
		      &w_ret,&h_ret,&b_ret,&xhot_ret,&yhot_ret)) {
    printf("probleme pour lire defil.bitmap\n");
    exit(-1);
  }
  pix_defil=XCreatePixmap(fl_display, FL_ObjWin(FD->browser), w_ret,h_ret,
			  DefaultDepth(fl_display,DefaultScreen(fl_display)));
  XCopyPlane(fl_display, b_ret,pix_defil, gc, 0, 0, w_ret, h_ret, 0, 0, 1L);
  XFreePixmap(fl_display, b_ret);

  pix_w = w_ret;
  pix_h = h_ret;

  defil_xoffset = fd_mpcview->browser->x +
    (fd_mpcview->browser->w - pix_w)/2;
  defil_yoffset = fd_mpcview->browser->y +
    (fd_mpcview->browser->h - pix_h*2)/2;

  defil_sens = defil_cpt_sens = 0;
  defil_disp = fl_display;

  defil_dest = FL_ObjWin(FD->browser);
  defil_haut = pix_h;
  for (i=0; i < defil_haut; i++) defil_ligne[i]=i;

  /* boucle d'évènements */

  {
    int i;
    for (i = 0; i < NPOINTS; i++) {
      scan_x[i] = i;
      scan_y[i] = 0;
    }

  fl_set_xyplot_xbounds(FD->scan, 0, NPOINTS - 1);
  fl_set_xyplot_ybounds(FD->scan, 0, YMAX);
  fl_set_xyplot_data(FD->scan, scan_x, scan_y, NPOINTS, "THROUGHPUT",
		     "seconds", "MB/s");
  }

  for (;;) {
    XEvent xev;
    FL_OBJECT *ob;

    ob = fl_do_forms();
    if (ob == FL_EVENT) fl_XNextEvent(&xev);

    if (ob == FD->menufile) {
      exit(0);
    }
  }

  close(fd);
  exit(0);
}

