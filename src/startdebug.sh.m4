#!/bin/sh
# $Id: startdebug.sh.m4,v 1.1.1.1 1998/10/28 21:07:35 alex Exp $

XHOST + HOST1
XHOST + HOST2

RSH -n HOST1 'XTERM -T HOST1 -geometry 80x11+3+2 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST1 'XTERM -T HOST1 -geometry 80x11+513+2 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST1 'XTERM -T HOST1 -geometry 80x11+3+180 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST1 'XTERM -T HOST1 -geometry 80x11+513+180 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &

RSH -n HOST2 'XTERM -T HOST2 -geometry 80x11+3+358 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST2 'XTERM -T HOST2 -geometry 80x11+513+358 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST2 'XTERM -T HOST2 -geometry 80x11+3+536 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &
RSH -n HOST2 'XTERM -T HOST2 -geometry 80x11+513+536 -display HOSTNAME:0 -e CSH -c "cd WHERE; SHELL"' &

