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

# $Id: Makefile.am,v 1.3 2000/03/08 21:45:22 alex Exp $

# IT WORKS (for gcc) :
# CFLAGS += foo

# -D_THREAD_SAFE pour redefinir une macro a la place de errno

# CXXFLAGS must NOT contain -O2 (it would generate an internal gcc error)


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

CXXFLAGS = -g -I/usr/include -I/sys -D_SMP_SUPPORT
CFLAGS =  -g -I/usr/include -I/sys -D_SMP_SUPPORT

SUFFIXES = .cc
bin_PROGRAMS = vmd testlibmpc testlibmpc2 testlibmpc3                testlibmpc_read testlibmpc_write testlibmpc_select


vmd_SOURCES = vmd.cc distobj.cc vmspace.cc object.cc typeid.cc

testlibmpc_LDADD = libmpc.a
testlibmpc2_LDADD = libmpc.a
testlibmpc3_LDADD = libmpc.a
testlibmpc_read_LDADD = libmpc.a
testlibmpc_write_LDADD = libmpc.a
testlibmpc_select_LDADD = libmpc.a

lib_LIBRARIES = libmpc.a
include_HEADERS = mpc.h mpcshare.h

libmpc_a_SOURCES = mpc.c

# Don't want to use libtool...
pkgdata_DATA = libsocketwrap.so.1.0 libmpc.so.1.0

EXTRA_DIST = transcript.pl vmd.h distobj.h distobj.dist.h object.h              typeid.h vmspace.h socketwrap.c stopvmd.sh loader.sh


CLEANFILES = libmpc.so.1.0 libsocketwrap.so.1.0 distobj.h
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
libmpc_a_LIBADD = 
libmpc_a_OBJECTS =  mpc.o
PROGRAMS =  $(bin_PROGRAMS)

vmd_OBJECTS =  vmd.o distobj.o vmspace.o object.o typeid.o
vmd_LDADD = $(LDADD)
vmd_DEPENDENCIES = 
vmd_LDFLAGS = 
testlibmpc_SOURCES = testlibmpc.c
testlibmpc_OBJECTS =  testlibmpc.o
testlibmpc_DEPENDENCIES =  libmpc.a
testlibmpc_LDFLAGS = 
testlibmpc2_SOURCES = testlibmpc2.c
testlibmpc2_OBJECTS =  testlibmpc2.o
testlibmpc2_DEPENDENCIES =  libmpc.a
testlibmpc2_LDFLAGS = 
testlibmpc3_SOURCES = testlibmpc3.c
testlibmpc3_OBJECTS =  testlibmpc3.o
testlibmpc3_DEPENDENCIES =  libmpc.a
testlibmpc3_LDFLAGS = 
testlibmpc_read_SOURCES = testlibmpc_read.c
testlibmpc_read_OBJECTS =  testlibmpc_read.o
testlibmpc_read_DEPENDENCIES =  libmpc.a
testlibmpc_read_LDFLAGS = 
testlibmpc_write_SOURCES = testlibmpc_write.c
testlibmpc_write_OBJECTS =  testlibmpc_write.o
testlibmpc_write_DEPENDENCIES =  libmpc.a
testlibmpc_write_LDFLAGS = 
testlibmpc_select_SOURCES = testlibmpc_select.c
testlibmpc_select_OBJECTS =  testlibmpc_select.o
testlibmpc_select_DEPENDENCIES =  libmpc.a
testlibmpc_select_LDFLAGS = 
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DATA =  $(pkgdata_DATA)

HEADERS =  $(include_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
DEP_FILES =  .deps/distobj.P .deps/mpc.P .deps/object.P \
.deps/testlibmpc.P .deps/testlibmpc2.P .deps/testlibmpc3.P \
.deps/testlibmpc_read.P .deps/testlibmpc_select.P \
.deps/testlibmpc_write.P .deps/typeid.P .deps/vmd.P .deps/vmspace.P
SOURCES = $(libmpc_a_SOURCES) $(vmd_SOURCES) testlibmpc.c testlibmpc2.c testlibmpc3.c testlibmpc_read.c testlibmpc_write.c testlibmpc_select.c
OBJECTS = $(libmpc_a_OBJECTS) $(vmd_OBJECTS) testlibmpc.o testlibmpc2.o testlibmpc3.o testlibmpc_read.o testlibmpc_write.o testlibmpc_select.o

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .cc .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu MPC_OS/Makefile

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

libmpc.a: $(libmpc_a_OBJECTS) $(libmpc_a_DEPENDENCIES)
	-rm -f libmpc.a
	$(AR) cru libmpc.a $(libmpc_a_OBJECTS) $(libmpc_a_LIBADD)
	$(RANLIB) libmpc.a

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

testlibmpc: $(testlibmpc_OBJECTS) $(testlibmpc_DEPENDENCIES)
	@rm -f testlibmpc
	$(LINK) $(testlibmpc_LDFLAGS) $(testlibmpc_OBJECTS) $(testlibmpc_LDADD) $(LIBS)

testlibmpc2: $(testlibmpc2_OBJECTS) $(testlibmpc2_DEPENDENCIES)
	@rm -f testlibmpc2
	$(LINK) $(testlibmpc2_LDFLAGS) $(testlibmpc2_OBJECTS) $(testlibmpc2_LDADD) $(LIBS)

testlibmpc3: $(testlibmpc3_OBJECTS) $(testlibmpc3_DEPENDENCIES)
	@rm -f testlibmpc3
	$(LINK) $(testlibmpc3_LDFLAGS) $(testlibmpc3_OBJECTS) $(testlibmpc3_LDADD) $(LIBS)

testlibmpc_read: $(testlibmpc_read_OBJECTS) $(testlibmpc_read_DEPENDENCIES)
	@rm -f testlibmpc_read
	$(LINK) $(testlibmpc_read_LDFLAGS) $(testlibmpc_read_OBJECTS) $(testlibmpc_read_LDADD) $(LIBS)

testlibmpc_write: $(testlibmpc_write_OBJECTS) $(testlibmpc_write_DEPENDENCIES)
	@rm -f testlibmpc_write
	$(LINK) $(testlibmpc_write_LDFLAGS) $(testlibmpc_write_OBJECTS) $(testlibmpc_write_LDADD) $(LIBS)

testlibmpc_select: $(testlibmpc_select_OBJECTS) $(testlibmpc_select_DEPENDENCIES)
	@rm -f testlibmpc_select
	$(LINK) $(testlibmpc_select_LDFLAGS) $(testlibmpc_select_OBJECTS) $(testlibmpc_select_LDADD) $(LIBS)
.cc.o:
	$(CXXCOMPILE) -c $<

install-pkgdataDATA: $(pkgdata_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	@list='$(pkgdata_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgdatadir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgdatadir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(pkgdatadir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(pkgdatadir)/$$p; \
	  fi; fi; \
	done

uninstall-pkgdataDATA:
	@$(NORMAL_UNINSTALL)
	list='$(pkgdata_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(pkgdatadir)/$$p; \
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

subdir = MPC_OS

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu MPC_OS/Makefile
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

%.o: %.cc
	@echo '$(CXXCOMPILE) -c $<'; \
	$(CXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
	@-cp .deps/$(*F).pp .deps/$(*F).P; \
	tr ' ' '\012' < .deps/$(*F).pp \
	  | sed -e 's/^\\$$//' -e '/^$$/ d' -e '/:$$/ d' -e 's/$$/ :/' \
	    >> .deps/$(*F).P; \
	rm .deps/$(*F).pp

%.lo: %.cc
	@echo '$(LTCXXCOMPILE) -c $<'; \
	$(LTCXXCOMPILE) -Wp,-MD,.deps/$(*F).pp -c $<
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
install-exec-am: install-libLIBRARIES install-binPROGRAMS
install-exec: install-exec-am

install-data-am: install-pkgdataDATA install-includeHEADERS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-libLIBRARIES uninstall-binPROGRAMS \
		uninstall-pkgdataDATA uninstall-includeHEADERS
uninstall: uninstall-am
all-am: Makefile $(LIBRARIES) $(PROGRAMS) $(DATA) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(libdir) $(DESTDIR)$(bindir) \
		$(DESTDIR)$(pkgdatadir) $(DESTDIR)$(includedir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
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
uninstall-binPROGRAMS install-binPROGRAMS uninstall-pkgdataDATA \
install-pkgdataDATA uninstall-includeHEADERS install-includeHEADERS \
tags mostlyclean-tags distclean-tags clean-tags maintainer-clean-tags \
distdir mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


# un de ces deux la doit apparaitre, sinon on considere que c'est sans
# buffering (mais avec un stdiobuf)
# -DWITH_STDIOBUF -DWITH_FILEBUF

typeid.o: typeid.cc typeid.h
	$(CXX) -fhandle-exceptions -Wall -frtti -c -ansi -foperator-names \
	-D_THREADSAFE $(CXXFLAGS) typeid.cc

vmd.o: vmd.cc vmd.h distobj.h
	$(CXX) -DWITH_COMMAND_LINE_THREAD -fhandle-exceptions -Wall -c -ansi \
	-foperator-names -D_THREAD_SAFE $(CXXFLAGS) vmd.cc

distobj.h: distobj.dist.h
	$(PERL) ./transcript.pl < distobj.dist.h > distobj.h

distobj.o: distobj.cc distobj.h vmd.h
	$(CXX) -fhandle-exceptions -Wall -c -ansi -foperator-names \
	-D_THREAD_SAFE $(CXXFLAGS) distobj.cc

vmspace.o: vmspace.cc
	$(CXX) -fhandle-exceptions -c -ansi -D_THREAD_SAFE $(CXXFLAGS) \
	vmspace.cc

object.o: object.cc
	$(CXX) -fhandle-exceptions -c -ansi -D_THREAD_SAFE $(CXXFLAGS) \
	object.cc

vmd: ${vmd_OBJECTS}
	if /bin/test "X-D_SMP_SUPPORTY" = XY ; \
	then \
	  $(CXX) -nostdlib -e start -o vmd vmd.o distobj.o vmspace.o object.o \
	  typeid.o /usr/lib/crt0.o -lg++ -lstdc++ -lm /usr/lib/libgcc.a -lc_r ; \
	else \
	  $(LD) -m elf_i386 -dynamic-linker /usr/libexec/ld-elf.so.1 \
	   -o vmd /usr/lib/crt1.o /usr/lib/crti.o /usr/lib/crtbegin.o \
	   -L/usr/libdata/gcc vmd.o distobj.o vmspace.o object.o typeid.o \
	   -lg++ -lstdc++ -lm /usr/lib/libgcc.a -lc_r /usr/lib/libgcc.a \
	   /usr/lib/crtend.o /usr/lib/crtn.o ; \
	fi

# c++rt0.o must be linked against the library in order to call constructors
# at program startup, when compiling into a.out format. No such need when
# compiling in elf format.
libsocketwrap.so.1.0: socketwrap.o mpc-pic.o
	if /bin/test "X-D_SMP_SUPPORTY" = XY ; \
	then \
	  $(LD) -Bshareable -o libsocketwrap.so.1.0 /usr/lib/c++rt0.o \
	  socketwrap.o mpc-pic.o ; \
	else \
	  $(LD) -Bshareable -o libsocketwrap.so.1.0 \
	  socketwrap.o mpc-pic.o ; \
	fi

# c++rt0.o must be linked against the library in order to call constructors
# at program startup, when compiling into a.out format. No such need when
# compiling in elf format.
libmpc.so.1.0: mpc-pic.o
	if /bin/test "X-D_SMP_SUPPORTY" = XY ; \
	then \
	  $(LD) -Bshareable -o libmpc.so.1.0 /usr/lib/c++rt0.o mpc-pic.o ; \
	else \
	  $(LD) -Bshareable -o libmpc.so.1.0 mpc-pic.o ; \
	fi

socketwrap.o: socketwrap.c
	$(CC) -fPIC  -DMPC_STATS $(CFLAGS) -c socketwrap.c

mpc-pic.o: mpc.c mpc.h mpcshare.h
	$(CC) -fPIC  -DMPC_STATS $(CFLAGS) -o mpc-pic.o -c mpc.c

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
