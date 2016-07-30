#! /bin/zsh -f
PATH=/usr/bin:/usr/local/bin:$PATH

# Do make clean
rm -rf **/.deps
rm config.status config.guess config.sub install-sh missing mkinstalldirs aclocal.m4
./aclocal
automake -a -c
autoconf
rm config.cache
./configure --disable-debug --disable-soft-workaround
gmake clean

# clean all
rm -rf **/.deps
rm -rf **/Makefile
rm -rf config.status config.cache

# compilation
echo
echo 'WARNING : This package is designed for PCIDDC 2nd Run and BX-like PCI bridges.'
echo '          To use this package with another PCIDDC or another bridge,'
echo '          please contact alex@mpc.lip6.fr'
echo 'Now you should enter something like that :'
echo '(note that with SMP nodes, you must already run a SMP kernel'
echo 'prior to execute the next line)'
echo 'root# ./configure --disable-debug --disable-soft-workaround'
echo 'root# gmake'
