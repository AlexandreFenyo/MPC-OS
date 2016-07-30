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

# $Id: Makefile.am,v 1.2 1999/02/13 15:11:58 alex Exp $


SHELL = /bin/sh

srcdir = .
top_srcdir = .
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

top_builddir = .

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

SUBDIRS = src modules examples config_mpc MPC_OS mpcview routing # docs
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
DIST_COMMON =  README AUTHORS COPYING ChangeLog INSTALL Makefile.am \
Makefile.in NEWS TODO aclocal.m4 config.guess config.sub configure \
configure.in install-sh missing mkinstalldirs


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(ACLOCAL_M4):  configure.in 
	cd $(srcdir) && $(ACLOCAL)

config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck
$(srcdir)/configure: $(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" = "." && dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

tags: TAGS

TAGS: tags-recursive $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
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

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) dist
	-rm -rf $(distdir)
	@banner="$(distdir).tar.gz is ready for distribution"; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu Makefile
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
	for subdir in $(SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(distdir) distdir=../$(distdir)/$$subdir distdir) \
	      || exit 1; \
	  fi; \
	done
info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
install-exec-am:
install-exec: install-exec-recursive

install-data-am:
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am:
uninstall: uninstall-recursive
all-am: Makefile
all-redirect: all-recursive
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-tags mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-tags clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-tags distclean-generic clean-am

distclean: distclean-recursive
	-rm -f config.status

maintainer-clean-am:  maintainer-clean-tags maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive
	-rm -f config.status

.PHONY: install-data-recursive uninstall-data-recursive \
install-exec-recursive uninstall-exec-recursive installdirs-recursive \
uninstalldirs-recursive all-recursive check-recursive \
installcheck-recursive info-recursive dvi-recursive \
mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs-am \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean




load:
	@cd modules; $(MAKE) $@

loadcmem:
	@cd modules; $(MAKE) $@

loadserial:
	@cd modules; $(MAKE) $@

unload:
	@cd modules; $(MAKE) $@

unloadcmem:
	@cd modules; $(MAKE) $@

unloadserial:
	@cd modules; $(MAKE) $@

lines:
	/usr/bin/wc -l Makefile.am Makefile.in configure.in \
	MPC_OS/Makefile.am MPC_OS/Makefile.in MPC_OS/*.cc MPC_OS/object.h \
	MPC_OS/typeid.h MPC_OS/vmd.h MPC_OS/vmspace.h MPC_OS/distobj.dist.h \
	MPC_OS/transcript.pl MPC_OS/mpc.c MPC_OS/mpc.h MPC_OS/socketwrap.c \
	MPC_OS/testlibmpc2.c MPC_OS/testlibmpc3.c MPC_OS/testlibmpc_read.c \
	MPC_OS/testlibmpc_write.c MPC_OS/testlibmpc_select.c MPC_OS/testlibmpc_com.c \
	MPC_OS/testlibmpc.c MPC_OS/mpcshare.h MPC_OS/loader.sh \
	config_mpc/Makefile.am config_mpc/Makefile.in config_mpc/hsl.conf.m4 \
	docs/Makefile.am docs/Makefile.in \
	examples/Makefile.am examples/Makefile.in examples/*.c \
	src/Makefile.am src/Makefile.in src/*.c src/*.h src/*.x src/*.m4 \
	modules/Makefile.am modules/Makefile.in modules/*.c modules/*.h \
	mpcview/Makefile.am mpcview/Makefile.in mpcview/*.c mpcview/*.h \
	routing/autorouting.c routing/autorouting.h \
	routing/building.c routing/building.h \
	routing/config.c routing/config.h \
	routing/erreurs.c routing/erreurs.h \
	routing/generator_driver.c routing/router_driver.c \
	routing/generic.h routing/global.c \
	routing/globals.c routing/globals.h \
	routing/lslime.l routing/yslime.y routing/misc.c routing/misc.h \
	routing/moteur.c routing/rcube.c routing/reseau.c routing/reseau.h \
	routing/routage.c routing/routage.h \
	routing/Makefile.am routing/Makefile.in \
	config_mpc/Catalog config_mpc/config.ml \
	config_mpc/generator.mmd config_mpc/router.mmd \
	config_mpc/types.cfg routing/mle2raw.pl src/dumpPCIDDC.c

# 37527 lines
#
# 1259  scripts 3%
# 7503  C++    20%
# 28765 C      77%
#
# kernel (C) : 16324      44%
# outside (C++/scripts) : 56% demons, exemples, libraries, progs de debug, routage
#
# cmem :      4% of kernel
# driver:    19%
# put:       17%
# slrp/scpp: 32%
# slrv/scpv: 17%
# mdcp/select : 11%
#

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
