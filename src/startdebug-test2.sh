#!/bin/sh
# $Id: startdebug.sh.m4,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $

/usr/X11R6.3/bin/xhost + mpc1-lip6
/usr/X11R6.3/bin/xhost + mpc4-lip6

/usr/bin/rsh  -n mpc1-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc1-lip6 -geometry 80x11+3+2 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc1-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc1-lip6 -geometry 80x11+513+2 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc1-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc1-lip6 -geometry 80x11+3+180 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc1-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc1-lip6 -geometry 80x11+513+180 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &

/usr/bin/rsh  -n mpc4-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc4-lip6 -geometry 80x11+3+358 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc4-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc4-lip6 -geometry 80x11+513+358 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc4-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc4-lip6 -geometry 80x11+3+536 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &
/usr/bin/rsh  -n mpc4-lip6 '/usr/X11R6.3/bin/xterm -display pmpc6:0 -T mpc4-lip6 -geometry 80x11+513+536 -e /bin/csh -c "cd /eowynhome/alex/cvs/alex/MPC-OS-5/modules; /usr/local/bin/zsh"' &

