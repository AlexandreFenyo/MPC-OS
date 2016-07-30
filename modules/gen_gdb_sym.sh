#! /usr/local/bin/zsh
# $Id: gen_gdb_sym.sh,v 1.1 2000/02/15 19:03:50 alex Exp $

PATH=/bin:/usr/bin:$PATH

kldstat -n cmemdriver_mod | grep 0x | awk '{ print $3; }' | sed 's/^0x//' | tr a-f A-F | read BASE
objdump -h /modules/cmemdriver_mod.ko | grep ' .text' | awk '{ print $6; }' | tr a-f A-F | read OFFSET
(echo -n 0x; echo "obase=16;ibase=16;$BASE+$OFFSET" | bc | tr A-F a-f ) | read CMEMTXTADDR

kldstat -n hsldriver_mod | grep 0x | awk '{ print $3; }' | sed 's/^0x//' | tr a-f A-F | read BASE
objdump -h /modules/hsldriver_mod.ko | grep ' .text' | awk '{ print $6; }' | tr a-f A-F | read OFFSET
(echo -n 0x; echo "obase=16;ibase=16;$BASE+$OFFSET" | bc | tr A-F a-f ) | read HSLTXTADDR

(
echo "set height 0"
echo "file /kernel"
echo "cd /sys/i386/conf"
echo "add-symbol-file /modules/cmemdriver_mod.ko $CMEMTXTADDR"
echo "add-symbol-file /modules/hsldriver_mod.ko $HSLTXTADDR"
echo "target remote /dev/cuaa0"
echo "delete"
echo "break cmem.c:dev_write"
echo "info breakpoints"
) > gdb_remote.`hostname | sed 's/\..*//'`

