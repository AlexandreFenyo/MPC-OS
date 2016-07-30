#! /bin/sh
# $Id: stopvmd.sh,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $

/bin/ps auxgww | grep vmd | grep -v vmdaemon | grep -v grep | grep -v stopvmd

proclist=`ps auxgww | grep vmd | grep -v vmdaemon | grep -v grep | grep -v stopvmd | awk '{ print $2; }'`

/bin/kill -TERM $proclist
/bin/sleep 2
/bin/kill -9 $proclist

