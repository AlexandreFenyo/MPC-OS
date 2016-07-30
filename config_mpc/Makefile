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

# $Id: Makefile.am,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $


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

pkgdata_DATA = hsl-bench1.conf hsl-bench2.conf

CLEANFILES = hsl-bench1.conf hsl-bench2.conf

EXTRA_DIST = hsl.conf.m4 types.cfg Catalog config.ml generator.mmd router.mmd net-1-res.mle net-1.mle net-2-res.mle net-2.mle net-3-res.mle net-3.mle net-4-res.mle net-4.mle net-1-loopback-res.mle net-1-loopback.mle net-1-loopback-res.raw 1host.cluster example.cluster
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
DATA =  $(pkgdata_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu config_mpc/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


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
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = config_mpc

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu config_mpc/Makefile
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
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-pkgdataDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-pkgdataDATA
uninstall: uninstall-am
all-am: Makefile $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(pkgdatadir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-pkgdataDATA install-pkgdataDATA tags distdir info-am \
info dvi-am dvi check check-am installcheck-am installcheck \
install-exec-am install-exec install-data-am install-data install-am \
install uninstall-am uninstall all-redirect all-am all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


hsl-bench1.conf: hsl.conf.m4
	-if /bin/test Xbench1/bench2 = Xno ; \
	then                          \
	  exit 1 ;                    \
	fi
	/usr/bin/m4 -DNODE1=0 -DNODE2=1 -DHOSTNAME2=bench2 \
	              hsl.conf.m4 > hsl-bench1.conf

hsl-bench2.conf: hsl.conf.m4
	-if /bin/test Xbench1/bench2 = Xno ; \
	then                          \
	  exit 1 ;                    \
	fi
	/usr/bin/m4 -DNODE1=1 -DNODE2=0 -DHOSTNAME2=bench1 \
	              hsl.conf.m4 > hsl-bench2.conf

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
