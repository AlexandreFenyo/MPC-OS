#! /usr/local/bin/zsh -f
# $Id$
# ./symaddr.sh module.ko symbol

PATH=/bin:/usr/bin:$PATH

if [ $# -ne 2 ]
then
    echo "usage: $0 module.ko symbol"
    exit 1
fi

kldstat -n $1 | grep 0x | awk '{ print $3; }' | sed 's/^0x//' | tr a-f A-F | read BASE
nm -a /modules/$1 | grep " $2" | awk '{ print $1; }' | tr a-f A-F | read OFFSET
(echo -n 0x; echo "obase=16;ibase=16;$BASE+$OFFSET" | bc | tr A-F a-f ) | read SYMADDR

echo $SYMADDR
