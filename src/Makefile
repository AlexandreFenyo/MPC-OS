# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# $Id: Makefile.am,v 1.5 2000/02/23 18:11:25 alex Exp $


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/mpc
pkglibdir = $(libdir)/mpc
pkgincludedir = $(includedir)/mpc

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = /usr/local/bin/autoconf
AUTOMAKE = /usr/local/bin/automake
AUTOHEADER = /usr/local/bin/autoheader

INSTALL = @INSTALL@
INSTALL_PROGRAM = @INSTALL_PROGRAM@ $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = @INSTALL_DATA@
INSTALL_SCRIPT = @INSTALL_SCRIPT@
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i386-unknown-freebsd3.4
build_triplet = i386-unknown-freebsd3.4
host_alias = i386-unknown-freebsd3.4
host_triplet = i386-unknown-freebsd3.4
target_alias = i386-unknown-freebsd3.4
target_triplet = i386-unknown-freebsd3.4
AR = /usr/bin/ar
AS = /usr/bin/as
AUTOCONF = /usr/local/bin/autoconf
AUTOHEADER = /usr/local/bin/autoheader
AUTOMAKE = /usr/local/bin/automake
CAT = /bin/cat
CC = gcc
CHMOD = /bin/chmod
CP = /bin/cp
CPP = gcc -E
CSH = /bin/csh
CUT = /usr/bin/cut
CXX = c++
CXXCPP = c++ -E
DEBUG_FLAGS =  -DMPC_STATS
ECHO = /bin/echo
FALSE = /usr/bin/false
GENSETDEFS = /usr/bin/gensetdefs
GREP = /usr/bin/grep
HOST1 = bench1
HOST2 = bench2
HOSTNAME = /bin/hostname
ID = /usr/bin/id
KERNEL_FILE = /kernel
KLDLOAD = /sbin/kldload
KLDSTAT = /sbin/kldstat
KLDUNLOAD = /sbin/kldunload
LD = /usr/bin/ld
LEX = flex
LIBXFORMS = -lxforms
LIMITS = /usr/bin/limits
LN_S = ln -s
LOOPBACK = 
LS = /bin/ls
M4 = /usr/bin/m4
MAKEDEPEND = /usr/X11R6.3/bin/makedepend
MAKEINFO = /usr/bin/makeinfo
MKDIR = /bin/mkdir
MKNOD = /sbin/mknod
MODLOAD = /sbin/modload
MODSTAT = /usr/bin/modstat
MODUNLOAD = /sbin/modunload
MV = /bin/mv
NM = /usr/bin/nm
PACKAGE = mpc
PERL = /usr/bin/perl5
PWD = /eowyn_home/alex/cvs/alex/MPC-OS-5
RANLIB = ranlib
RM = /bin/rm
RPCGEN = /usr/bin/rpcgen
RSH = /usr/bin/rsh
SED = /usr/bin/sed
SMP_FLAGS_1 = -I/usr/include -I/sys
SMP_FLAGS_2 = -D_SMP_SUPPORT
SMP_FLAGS_3 = -elf
TEST = /bin/test
TOUCH = /usr/bin/touch
TR = /usr/bin/tr
TRUE = /usr/bin/true
VERSION = 5.3
WC = /usr/bin/wc
WITH_KLD = yes
WITH_STABS = yes
WORKAROUND_FLAGS = -DPCIDDC2ndRun
XHOST = /usr/X11R6.3/bin/xhost
XTERM = /usr/X11R6.3/bin/xterm
X_HOSTS = bench1/bench2
X_SHELL = /usr/local/bin/zsh
YACC = /usr/bin/yacc
ZSH = /usr/local/bin/zsh

bin_PROGRAMS = hslclient hslserver malloc virt2phys readphys writephys 	       getevents vmconst init_pcibus readpci writepci vmspace 	       dumpPCIDDC setmode testputrecv testputsend testputbench 	       dumptables prot_dump prot_wire prot_garbcoll msqsend 	       socksend demo hsl2tty testscp mmapbench dumpchannelstate 	       selectnotice testputsend_loop bench_latence bench_throughput 	       testtimer checkcalibration simulateEEP dumpINTR


bin_SCRIPTS = startdebug.sh startdebug-vmd.sh

lib_LIBRARIES = libuseraccess.a
include_HEADERS = useraccess.h

libuseraccess_a_SOURCES = useraccess.c

init_pcibus_LDADD = -lkvm

CFLAGS = ${DEFS} -g ${OPTFLAGS} -Wall  -DMPC_STATS -O2 -I/usr/include -I/sys

hslclient_SOURCES = hslclient.c rpctransport_clnt.c

hslserver_SOURCES = hslserver.c rpctransport_svc.c

BUILT_SOURCES = rpctransport_xdr.h rpctransport.h

testputrecv_LDADD = libuseraccess.a

testputsend_LDADD = libuseraccess.a

testputsend_loop_LDADD = lubuseraccess.a

testputbench_LDADD = libuseraccess.a

bench_latence_LDADD = libuseraccess.a

bench_throughput_LDADD = libuseraccess.a

CLEANFILES = ${BUILT_SOURCES} rpctransport_clnt.c rpctransport_svc.c              rpctransport_xdr.c startdebug.sh startdebug-vmd.sh


EXTRA_DIST = hslclient.h hslserver.h rpctransport.x startdebug.sh.m4              startdebug-vmd.sh.m4

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(lib_LIBRARIES)


DEFS =  -DYYTEXT_POINTER=1 -DSTDC_HEADERS=1 -DKERNEL_FILE=\"/kernel\" -D_SMP_SUPPORT=1 -DWITH_STABS_PCIBUS_SET=1 -DWITH_KLD=1 -D_WITHOUT_DDSCP=1 -D_WITHOUT_MONITOR=1 -DWITHOUT_LOOPBACK=1  -I. -I$(srcdir) 
CPPFLAGS = 
LDFLAGS = 
LIBS = 
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
libuseraccess_a_LIBADD = 
libuseraccess_a_OBJECTS =  useraccess.o
PROGRAMS =  $(bin_PROGRAMS)

hslclient_OBJECTS =  hslclient.o rpctransport_clnt.o
hslclient_LDADD = $(LDADD)
hslclient_DEPENDENCIES = 
hslclient_LDFLAGS = 
hslserver_OBJECTS =  hslserver.o rpctransport_svc.o
hslserver_LDADD = $(LDADD)
hslserver_DEPENDENCIES = 
hslserver_LDFLAGS = 
malloc_SOURCES = malloc.c
malloc_OBJECTS =  malloc.o
malloc_LDADD = $(LDADD)
malloc_DEPENDENCIES = 
malloc_LDFLAGS = 
virt2phys_SOURCES = virt2phys.c
virt2phys_OBJECTS =  virt2phys.o
virt2phys_LDADD = $(LDADD)
virt2phys_DEPENDENCIES = 
virt2phys_LDFLAGS = 
readphys_SOURCES = readphys.c
readphys_OBJECTS =  readphys.o
readphys_LDADD = $(LDADD)
readphys_DEPENDENCIES = 
readphys_LDFLAGS = 
writephys_SOURCES = writephys.c
writephys_OBJECTS =  writephys.o
writephys_LDADD = $(LDADD)
writephys_DEPENDENCIES = 
writephys_LDFLAGS = 
getevents_SOURCES = getevents.c
getevents_OBJECTS =  getevents.o
getevents_LDADD = $(LDADD)
getevents_DEPENDENCIES = 
getevents_LDFLAGS = 
vmconst_SOURCES = vmconst.c
vmconst_OBJECTS =  vmconst.o
vmconst_LDADD = $(LDADD)
vmconst_DEPENDENCIES = 
vmconst_LDFLAGS = 
init_pcibus_SOURCES = init_pcibus.c
init_pcibus_OBJECTS =  init_pcibus.o
init_pcibus_DEPENDENCIES = 
init_pcibus_LDFLAGS = 
readpci_SOURCES = readpci.c
readpci_OBJECTS =  readpci.o
readpci_LDADD = $(LDADD)
readpci_DEPENDENCIES = 
readpci_LDFLAGS = 
writepci_SOURCES = writepci.c
writepci_OBJECTS =  writepci.o
writepci_LDADD = $(LDADD)
writepci_DEPENDENCIES = 
writepci_LDFLAGS = 
vmspace_SOURCES = vmspace.c
vmspace_OBJECTS =  vmspace.o
vmspace_LDADD = $(LDADD)
vmspace_DEPENDENCIES = 
vmspace_LDFLAGS = 
dumpPCIDDC_SOURCES = dumpPCIDDC.c
dumpPCIDDC_OBJECTS =  dumpPCIDDC.o
dumpPCIDDC_LDADD = $(LDADD)
dumpPCIDDC_DEPENDENCIES = 
dumpPCIDDC_LDFLAGS = 
setmode_SOURCES = setmode.c
setmode_OBJECTS =  setmode.o
setmode_LDADD = $(LDADD)
setmode_DEPENDENCIES = 
setmode_LDFLAGS = 
testputrecv_SOURCES = testputrecv.c
testputrecv_OBJECTS =  testputrecv.o
testputrecv_DEPENDENCIES =  libuseraccess.a
testputrecv_LDFLAGS = 
testputsend_SOURCES = testputsend.c
testputsend_OBJECTS =  testputsend.o
testputsend_DEPENDENCIES =  libuseraccess.a
testputsend_LDFLAGS = 
testputbench_SOURCES = testputbench.c
testputbench_OBJECTS =  testputbench.o
testputbench_DEPENDENCIES =  libuseraccess.a
testputbench_LDFLAGS = 
dumptables_SOURCES = dumptables.c
dumptables_OBJECTS =  dumptables.o
dumptables_LDADD = $(LDADD)
dumptables_DEPENDENCIES = 
dumptables_LDFLAGS = 
prot_dump_SOURCES = prot_dump.c
prot_dump_OBJECTS =  prot_dump.o
prot_dump_LDADD = $(LDADD)
prot_dump_DEPENDENCIES = 
prot_dump_LDFLAGS = 
prot_wire_SOURCES = prot_wire.c
prot_wire_OBJECTS =  prot_wire.o
prot_wire_LDADD = $(LDADD)
prot_wire_DEPENDENCIES = 
prot_wire_LDFLAGS = 
prot_garbcoll_SOURCES = prot_garbcoll.c
prot_garbcoll_OBJECTS =  prot_garbcoll.o
prot_garbcoll_LDADD = $(LDADD)
prot_garbcoll_DEPENDENCIES = 
prot_garbcoll_LDFLAGS = 
msqsend_SOURCES = msqsend.c
msqsend_OBJECTS =  msqsend.o
msqsend_LDADD = $(LDADD)
msqsend_DEPENDENCIES = 
msqsend_LDFLAGS = 
socksend_SOURCES = socksend.c
socksend_OBJECTS =  socksend.o
socksend_LDADD = $(LDADD)
socksend_DEPENDENCIES = 
socksend_LDFLAGS = 
demo_SOURCES = demo.c
demo_OBJECTS =  demo.o
demo_LDADD = $(LDADD)
demo_DEPENDENCIES = 
demo_LDFLAGS = 
hsl2tty_SOURCES = hsl2tty.c
hsl2tty_OBJECTS =  hsl2tty.o
hsl2tty_LDADD = $(LDADD)
hsl2tty_DEPENDENCIES = 
hsl2tty_LDFLAGS = 
testscp_SOURCES = testscp.c
testscp_OBJECTS =  testscp.o
testscp_LDADD = $(LDADD)
testscp_DEPENDENCIES = 
testscp_LDFLAGS = 
mmapbench_SOURCES = mmapbench.c
mmapbench_OBJECTS =  mmapbench.o
mmapbench_LDADD = $(LDADD)
mmapbench_DEPENDENCIES = 
mmapbench_LDFLAGS = 
dumpchannelstate_SOURCES = dumpchannelstate.c
dumpchannelstate_OBJECTS =  dumpchannelstate.o
dumpchannelstate_LDADD = $(LDADD)
dumpchannelstate_DEPENDENCIES = 
dumpchannelstate_LDFLAGS = 
selectnotice_SOURCES = selectnotice.c
selectnotice_OBJECTS =  selectnotice.o
selectnotice_LDADD = $(LDADD)
selectnotice_DEPENDENCIES = 
selectnotice_LDFLAGS = 
testputsend_loop_SOURCES = testputsend_loop.c
testputsend_loop_OBJECTS =  testputsend_loop.o
testputsend_loop_DEPENDENCIES =  lubuseraccess.a
testputsend_loop_LDFLAGS = 
bench_latence_SOURCES = bench_latence.c
bench_latence_OBJECTS =  bench_latence.o
bench_latence_DEPENDENCIES =  libuseraccess.a
bench_latence_LDFLAGS = 
bench_throughput_SOURCES = bench_throughput.c
bench_throughput_OBJECTS =  bench_throughput.o
bench_throughput_DEPENDENCIES =  libuseraccess.a
bench_throughput_LDFLAGS = 
testtimer_SOURCES = testtimer.c
testtimer_OBJECTS =  testtimer.o
testtimer_LDADD = $(LDADD)
testtimer_DEPENDENCIES = 
testtimer_LDFLAGS = 
checkcalibration_SOURCES = checkcalibration.c
checkcalibration_OBJECTS =  checkcalibration.o
checkcalibration_LDADD = $(LDADD)
checkcalibration_DEPENDENCIES = 
checkcalibration_LDFLAGS = 
simulateEEP_SOURCES = simulateEEP.c
simulateEEP_OBJECTS =  simulateEEP.o
simulateEEP_LDADD = $(LDADD)
simulateEEP_DEPENDENCIES = 
simulateEEP_LDFLAGS = 
dumpINTR_SOURCES = dumpINTR.c
dumpINTR_OBJECTS =  dumpINTR.o
dumpINTR_LDADD = $(LDADD)
dumpINTR_DEPENDENCIES = 
dumpINTR_LDFLAGS = 
SCRIPTS =  $(bin_SCRIPTS)

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(include_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
DEP_FILES =  .deps/bench_latence.P .deps/bench_throughput.P \
.deps/checkcalibration.P .deps/demo.P .deps/dumpINTR.P \
.deps/dumpPCIDDC.P .deps/dumpchannelstate.P .deps/dumptables.P \
.deps/getevents.P .deps/hsl2tty.P .deps/hslclient.P .deps/hslserver.P \
.deps/init_pcibus.P .deps/malloc.P .deps/mmapbench.P .deps/msqsend.P \
.deps/prot_dump.P .deps/prot_garbcoll.P .deps/prot_wire.P \
.deps/readpci.P .deps/readphys.P .deps/rpctransport_clnt.P \
.deps/rpctransport_svc.P .deps/selectnotice.P .deps/setmode.P \
.deps/simulateEEP.P .deps/socksend.P .deps/testputbench.P \
.deps/testputrecv.P .deps/testputsend.P .deps/testputsend_loop.P \
.deps/testscp.P .deps/testtimer.P .deps/useraccess.P .deps/virt2phys.P \
.deps/vmconst.P .deps/vmspace.P .deps/writepci.P .deps/writephys.P
SOURCES = $(libuseraccess_a_SOURCES) $(hslclient_SOURCES) $(hslserver_SOURCES) malloc.c virt2phys.c readphys.c writephys.c getevents.c vmconst.c init_pcibus.c readpci.c writepci.c vmspace.c dumpPCIDDC.c setmode.c testputrecv.c testputsend.c testputbench.c dumptables.c prot_dump.c prot_wire.c prot_garbcoll.c msqsend.c socksend.c demo.c hsl2tty.c testscp.c mmapbench.c dumpchannelstate.c selectnotice.c testputsend_loop.c bench_latence.c bench_throughput.c testtimer.c checkcalibration.c simulateEEP.c dumpINTR.c
OBJECTS = $(libuseraccess_a_OBJECTS) $(hslclient_OBJECTS) $(hslserver_OBJECTS) malloc.o virt2phys.o readphys.o writephys.o getevents.o vmconst.o init_pcibus.o readpci.o writepci.o vmspace.o dumpPCIDDC.o setmode.o testputrecv.o testputsend.o testputbench.o dumptables.o prot_dump.o prot_wire.o prot_garbcoll.o msqsend.o socksend.o demo.o hsl2tty.o testscp.o mmapbench.o dumpchannelstate.o selectnotice.o testputsend_loop.o bench_latence.o bench_throughput.o testtimer.o checkcalibration.o simulateEEP.o dumpINTR.o

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu src/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-libLIBRARIES:

clean-libLIBRARIES:
	-test -z "$(lib_LIBRARIES)" || rm -f $(lib_LIBRARIES)

distclean-libLIBRARIES:

maintainer-clean-libLIBRARIES:

install-libLIBRARIES: $(lib_LIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(libdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done
	@$(POST_INSTALL)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(RANLIB) $(DESTDIR)$(libdir)/$$p"; \
	    $(RANLIB) $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done

uninstall-libLIBRARIES:
	@$(NORMAL_UNINSTALL)
	list='$(lib_LIBRARIES)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(libdir)/$$p; \
	done

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

libuseraccess.a: $(libuseraccess_a_OBJECTS) $(libuseraccess_a_DEPENDENCIES)
	-rm -f libuseraccess.a
	$(AR) cru libuseraccess.a $(libuseraccess_a_OBJECTS) $(libuseraccess_a_LIBADD)
	$(RANLIB) libuseraccess.a

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

hslclient: $(hslclient_OBJECTS) $(hslclient_DEPENDENCIES)
	@rm -f hslclient
	$(LINK) $(hslclient_LDFLAGS) $(hslclient_OBJECTS) $(hslclient_LDADD) $(LIBS)

hslserver: $(hslserver_OBJECTS) $(hslserver_DEPENDENCIES)
	@rm -f hslserver
	$(LINK) $(hslserver_LDFLAGS) $(hslserver_OBJECTS) $(hslserver_LDADD) $(LIBS)

malloc: $(malloc_OBJECTS) $(malloc_DEPENDENCIES)
	@rm -f malloc
	$(LINK) $(malloc_LDFLAGS) $(malloc_OBJECTS) $(malloc_LDADD) $(LIBS)

virt2phys: $(virt2phys_OBJECTS) $(virt2phys_DEPENDENCIES)
	@rm -f virt2phys
	$(LINK) $(virt2phys_LDFLAGS) $(virt2phys_OBJECTS) $(virt2phys_LDADD) $(LIBS)

readphys: $(readphys_OBJECTS) $(readphys_DEPENDENCIES)
	@rm -f readphys
	$(LINK) $(readphys_LDFLAGS) $(readphys_OBJECTS) $(readphys_LDADD) $(LIBS)

writephys: $(writephys_OBJECTS) $(writephys_DEPENDENCIES)
	@rm -f writephys
	$(LINK) $(writephys_LDFLAGS) $(writephys_OBJECTS) $(writephys_LDADD) $(LIBS)

getevents: $(getevents_OBJECTS) $(getevents_DEPENDENCIES)
	@rm -f getevents
	$(LINK) $(getevents_LDFLAGS) $(getevents_OBJECTS) $(getevents_LDADD) $(LIBS)

vmconst: $(vmconst_OBJECTS) $(vmconst_DEPENDENCIES)
	@rm -f vmconst
	$(LINK) $(vmconst_LDFLAGS) $(vmconst_OBJECTS) $(vmconst_LDADD) $(LIBS)

init_pcibus: $(init_pcibus_OBJECTS) $(init_pcibus_DEPENDENCIES)
	@rm -f init_pcibus
	$(LINK) $(init_pcibus_LDFLAGS) $(init_pcibus_OBJECTS) $(init_pcibus_LDADD) $(LIBS)

readpci: $(readpci_OBJECTS) $(readpci_DEPENDENCIES)
	@rm -f readpci
	$(LINK) $(readpci_LDFLAGS) $(readpci_OBJECTS) $(readpci_LDADD) $(LIBS)

writepci: $(writepci_OBJECTS) $(writepci_DEPENDENCIES)
	@rm -f writepci
	$(LINK) $(writepci_LDFLAGS) $(writepci_OBJECTS) $(writepci_LDADD) $(LIBS)

vmspace: $(vmspace_OBJECTS) $(vmspace_DEPENDENCIES)
	@rm -f vmspace
	$(LINK) $(vmspace_LDFLAGS) $(vmspace_OBJECTS) $(vmspace_LDADD) $(LIBS)

dumpPCIDDC: $(dumpPCIDDC_OBJECTS) $(dumpPCIDDC_DEPENDENCIES)
	@rm -f dumpPCIDDC
	$(LINK) $(dumpPCIDDC_LDFLAGS) $(dumpPCIDDC_OBJECTS) $(dumpPCIDDC_LDADD) $(LIBS)

setmode: $(setmode_OBJECTS) $(setmode_DEPENDENCIES)
	@rm -f setmode
	$(LINK) $(setmode_LDFLAGS) $(setmode_OBJECTS) $(setmode_LDADD) $(LIBS)

dumptables: $(dumptables_OBJECTS) $(dumptables_DEPENDENCIES)
	@rm -f dumptables
	$(LINK) $(dumptables_LDFLAGS) $(dumptables_OBJECTS) $(dumptables_LDADD) $(LIBS)

prot_dump: $(prot_dump_OBJECTS) $(prot_dump_DEPENDENCIES)
	@rm -f prot_dump
	$(LINK) $(prot_dump_LDFLAGS) $(prot_dump_OBJECTS) $(prot_dump_LDADD) $(LIBS)

prot_wire: $(prot_wire_OBJECTS) $(prot_wire_DEPENDENCIES)
	@rm -f prot_wire
	$(LINK) $(prot_wire_LDFLAGS) $(prot_wire_OBJECTS) $(prot_wire_LDADD) $(LIBS)

prot_garbcoll: $(prot_garbcoll_OBJECTS) $(prot_garbcoll_DEPENDENCIES)
	@rm -f prot_garbcoll
	$(LINK) $(prot_garbcoll_LDFLAGS) $(prot_garbcoll_OBJECTS) $(prot_garbcoll_LDADD) $(LIBS)

msqsend: $(msqsend_OBJECTS) $(msqsend_DEPENDENCIES)
	@rm -f msqsend
	$(LINK) $(msqsend_LDFLAGS) $(msqsend_OBJECTS) $(msqsend_LDADD) $(LIBS)

socksend: $(socksend_OBJECTS) $(socksend_DEPENDENCIES)
	@rm -f socksend
	$(LINK) $(socksend_LDFLAGS) $(socksend_OBJECTS) $(socksend_LDADD) $(LIBS)

demo: $(demo_OBJECTS) $(demo_DEPENDENCIES)
	@rm -f demo
	$(LINK) $(demo_LDFLAGS) $(demo_OBJECTS) $(demo_LDADD) $(LIBS)

hsl2tty: $(hsl2tty_OBJECTS) $(hsl2tty_DEPENDENCIES)
	@rm -f hsl2tty
	$(LINK) $(hsl2tty_LDFLAGS) $(hsl2tty_OBJECTS) $(hsl2tty_LDADD) $(LIBS)

testscp: $(testscp_OBJECTS) $(testscp_DEPENDENCIES)
	@rm -f testscp
	$(LINK) $(testscp_LDFLAGS) $(testscp_OBJECTS) $(testscp_LDADD) $(LIBS)

mmapbench: $(mmapbench_OBJECTS) $(mmapbench_DEPENDENCIES)
	@rm -f mmapbench
	$(LINK) $(mmapbench_LDFLAGS) $(mmapbench_OBJECTS) $(mmapbench_LDADD) $(LIBS)

dumpchannelstate: $(dumpchannelstate_OBJECTS) $(dumpchannelstate_DEPENDENCIES)
	@rm -f dumpchannelstate
	$(LINK) $(dumpchannelstate_LDFLAGS) $(dumpchannelstate_OBJECTS) $(dumpchannelstate_LDADD) $(LIBS)

selectnotice: $(selectnotice_OBJECTS) $(selectnotice_DEPENDENCIES)
	@rm -f selectnotice
	$(LINK) $(selectnotice_LDFLAGS) $(selectnotice_OBJECTS) $(selectnotice_LDADD) $(LIBS)

testtimer: $(testtimer_OBJECTS) $(testtimer_DEPENDENCIES)
	@rm -f testtimer
	$(LINK) $(testtimer_LDFLAGS) $(testtimer_OBJECTS) $(testtimer_LDADD) $(LIBS)

checkcalibration: $(checkcalibration_OBJECTS) $(checkcalibration_DEPENDENCIES)
	@rm -f checkcalibration
	$(LINK) $(checkcalibration_LDFLAGS) $(checkcalibration_OBJECTS) $(checkcalibration_LDADD) $(LIBS)

simulateEEP: $(simulateEEP_OBJECTS) $(simulateEEP_DEPENDENCIES)
	@rm -f simulateEEP
	$(LINK) $(simulateEEP_LDFLAGS) $(simulateEEP_OBJECTS) $(simulateEEP_LDADD) $(LIBS)

dumpINTR: $(dumpINTR_OBJECTS) $(dumpINTR_DEPENDENCIES)
	@rm -f dumpINTR
	$(LINK) $(dumpINTR_LDFLAGS) $(dumpINTR_OBJECTS) $(dumpINTR_LDADD) $(LIBS)

install-binSCRIPTS: $(bin_SCRIPTS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_SCRIPTS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(INSTALL_SCRIPT) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; fi; \
	done

uninstall-binSCRIPTS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_SCRIPTS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	done

install-includeHEADERS: $(include_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(includedir)
	@list='$(include_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(includedir)/$$p; \
	done

uninstall-includeHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(include_HEADERS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(includedir)/$$p; \
	done

tags: TAGS

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = src

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu src/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-hook

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)

-include $(DEP_FILES)

mostlyclean-depend:

clean-depend:

distclean-depend:
	-rm -rf .deps

maintainer-clean-depend:

%.o: %.c
	@echo '$(COMPILE) -c $<'; \
	$(COMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.c
	@echo '$(LTCOMPILE) -c $<'; \
	$(LTCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-sed -e 's/^\([^:]*\)\.o[ 	]*:/\1.lo \1.o :/' \
	  < .deps/$(*F).pp > .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm -f .deps/$(*F).pp
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-libLIBRARIES install-binPROGRAMS \
		install-binSCRIPTS
install-exec: install-exec-am

install-data-am: install-includeHEADERS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-libLIBRARIES uninstall-binPROGRAMS \
		uninstall-binSCRIPTS uninstall-includeHEADERS
uninstall: uninstall-am
all-am: Makefile $(LIBRARIES) $(PROGRAMS) $(SCRIPTS) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(libdir) $(DESTDIR)$(bindir) \
		$(DESTDIR)$(bindir) $(DESTDIR)$(includedir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
	-test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean-am:  mostlyclean-libLIBRARIES mostlyclean-compile \
		mostlyclean-binPROGRAMS mostlyclean-tags \
		mostlyclean-depend mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-libLIBRARIES clean-compile clean-binPROGRAMS clean-tags \
		clean-depend clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-libLIBRARIES distclean-compile \
		distclean-binPROGRAMS distclean-tags distclean-depend \
		distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-libLIBRARIES \
		maintainer-clean-compile maintainer-clean-binPROGRAMS \
		maintainer-clean-tags maintainer-clean-depend \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-libLIBRARIES distclean-libLIBRARIES \
clean-libLIBRARIES maintainer-clean-libLIBRARIES uninstall-libLIBRARIES \
install-libLIBRARIES mostlyclean-compile distclean-compile \
clean-compile maintainer-clean-compile mostlyclean-binPROGRAMS \
distclean-binPROGRAMS clean-binPROGRAMS maintainer-clean-binPROGRAMS \
uninstall-binPROGRAMS install-binPROGRAMS uninstall-binSCRIPTS \
install-binSCRIPTS uninstall-includeHEADERS install-includeHEADERS tags \
mostlyclean-tags distclean-tags clean-tags maintainer-clean-tags \
distdir mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean

rpctransport_clnt.c: rpctransport.x
	/usr/bin/rpcgen $?
rpctransport_svc.c: rpctransport.x
	/usr/bin/rpcgen $?

rpctransport_xdr.h: rpctransport.x
	/usr/bin/rpcgen -c $? > $@

rpctransport.h: rpctransport.x
	/usr/bin/rpcgen $?

startdebug.sh: startdebug.sh.m4
	if /bin/test bench1/bench2 != no ; \
	then \
	  /usr/bin/m4 -DHOST1=bench1 -DHOST2=bench2 -DXHOST=/usr/X11R6.3/bin/xhost \
	       -DRSH=/usr/bin/rsh -DXTERM=/usr/X11R6.3/bin/xterm -DHOSTNAME=`/bin/hostname` \
	       -DCSH=/bin/csh -DWHERE=/eowyn_home/alex/cvs/alex/MPC-OS-5/modules -DSHELL=/usr/local/bin/zsh \
	       startdebug.sh.m4 > startdebug.sh ; \
	  /bin/chmod +x startdebug.sh ; \
	fi

startdebug-vmd.sh: startdebug-vmd.sh.m4
	if /bin/test bench1/bench2 != no ; \
	then \
	  /usr/bin/m4 -DHOST1=bench1 -DHOST2=bench2 -DXHOST=/usr/X11R6.3/bin/xhost \
	       -DRSH=/usr/bin/rsh -DXTERM=/usr/X11R6.3/bin/xterm -DHOSTNAME=`/bin/hostname` \
	       -DCSH=/bin/csh -DWHERE=/eowyn_home/alex/cvs/alex/MPC-OS-5/MPC_OS -DSHELL=/usr/local/bin/zsh \
	       startdebug-vmd.sh.m4 > startdebug-vmd.sh ; \
	  /bin/chmod +x startdebug-vmd.sh ; \
	fi
testputrecv: libuseraccess.a
testputsend: libuseraccess.a
testputsend_loop: libuseraccess.a
testputbench: libuseraccess.a
bench_latence: libuseraccess.a
bench_throughput: libuseraccess.a

# makedepend doesn't like when a header file is included in a source file
# but doesn't exist...
dist-hook:
	-/bin/rm $(distdir)/rpctransport.h
#	/usr/bin/touch $(distdir)/rpctransport.h
#	sleep 1
#	/usr/bin/touch $(distdir)/rpctransport.x # this line to force rpctransport.h
	                                  # to be built

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
