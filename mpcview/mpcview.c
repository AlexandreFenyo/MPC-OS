/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "mpcview.h"

FD_mpcview *create_form_mpcview(void)
{
  FL_OBJECT *obj;
  FD_mpcview *fdui = (FD_mpcview *) fl_calloc(1, sizeof(*fdui));

  fdui->mpcview = fl_bgn_form(FL_NO_BOX, 425, 535);
  fdui->formmpcview = obj = fl_add_box(FL_FLAT_BOX,0,0,425,535,"");
  fdui->scanchart = obj = fl_add_chart(FL_LINE_CHART,15,40,280,60,"");
    fl_set_object_boxtype(obj,FL_UP_BOX);
  fdui->scan = obj = fl_add_xyplot(FL_FILL_XYPLOT,5,35,300,140,"");
    fl_set_object_boxtype(obj,FL_UP_BOX);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,5,345,415,155,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,245,355,165,80,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,15,355,225,135,"");
  fdui->button_err_0 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,365,90,20,"Exceptional EP");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,5,180,415,160,"");
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER,10,200,325,135,"");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
  fdui->menubar = obj = fl_add_box(FL_UP_BOX,0,0,425,30,"");
  fdui->clock = obj = fl_add_clock(FL_ANALOG_CLOCK,345,35,75,65,"");
  fdui->logo = obj = fl_add_pixmap(FL_NORMAL_PIXMAP,345,105,70,70,"");

  fdui->all_objects = fl_bgn_group();
  fdui->menufile = obj = fl_add_menu(FL_PULLDOWN_MENU,5,5,40,20,"File");
    fl_set_object_shortcut(obj,"#F",1);
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_gravity(obj, FL_North, FL_NoGravity);
  obj = fl_add_text(FL_NORMAL_TEXT,10,185,325,15,"This software is part of the MPC Parallel Machine project");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->average = obj = fl_add_valslider(FL_VERT_FILL_SLIDER,310,35,30,140,"");
    fl_set_object_color(obj,FL_COL1,FL_GREEN);
     fl_set_slider_return(obj, FL_RETURN_CHANGED);
  obj = fl_add_text(FL_NORMAL_TEXT,5,505,415,25,"Fast HSL Control Panel -- (c) LIP6/UPMC 1997-1998");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_group();

  fdui->button_0 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,340,200,35,30,"0");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_2 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,340,270,35,30,"2");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_1 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,340,235,35,30,"1");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_3 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,340,305,35,30,"3");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  obj = fl_add_text(FL_NORMAL_TEXT,265,5,100,20,"MPC-View");
    fl_set_object_lcol(obj,FL_DARKTOMATO);
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE+FL_EMBOSSED_STYLE);
  fdui->button_4 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,380,200,35,30,"4");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_5 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,380,235,35,30,"5");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_6 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,380,270,35,30,"6");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  fdui->button_7 = obj = fl_add_lightbutton(FL_PUSH_BUTTON,380,305,35,30,"7");
    fl_set_object_color(obj,FL_RED,FL_GREEN);
  obj = fl_add_text(FL_NORMAL_TEXT,30,350,40,15,"Errors");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->button_err_1 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,385,75,20,"CRC Header");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_2 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,405,70,20,"CRC Data");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_3 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,425,85,20,"End of Packet");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_4 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,445,70,20,"Timeout");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_5 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,20,465,75,20,"R3 Status");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_6 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,365,130,20,"Page Transmitted Overflow");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_7 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,385,130,20,"Message Received Overflow");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_8 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,405,120,20,"Sent Packet Overflow");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_9 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,425,130,20,"Received Packet Overflow");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_10 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,445,100,20,"Illegal LRM Access");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_err_11 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,105,465,80,20,"LMI Overflow");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT,325,370,75,15,"Tx/Rx Balance");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,325,405,75,15,"Packet Length");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,255,350,45,15,"Control");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,245,440,165,50,"");
  fdui->button_other_0 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,250,445,80,20,"Tx");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_other_1 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,250,465,80,20,"Rx");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_other_2 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,325,445,80,20,"RCube");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->button_other_3 = obj = fl_add_checkbutton(FL_PUSH_BUTTON,325,465,80,20,"Loopback");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
  fdui->tx_rx = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,250,365,75,25,"");
     fl_set_slider_return(obj, FL_RETURN_CHANGED);
  fdui->pack_len = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,250,400,75,25,"");
     fl_set_slider_return(obj, FL_RETURN_CHANGED);
  fl_end_form();

  return fdui;
}
/*---------------------------------------*/

