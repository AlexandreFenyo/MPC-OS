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

# $Id: Makefile.am,v 1.1.1.1 1998/10/28 21:07:34 alex Exp $


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

bin_PROGRAMS = ir

pkgdata_DATA = router_driver.o generator_driver.o moteur.o

ir_SOURCES = autorouting.c building.c config.c erreurs.c global.c              globals.c misc.c rcube.c reseau.c routage.c yslime.y lslime.l


ir_LDADD = -lfl

EXTRA_DIST = autorouting.h building.h config.h erreurs.h generic.h globals.h misc.h 	     reseau.h routage.h yslime.h router_driver.c generator_driver.c moteur.c

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(bin_PROGRAMS)


DEFS =  -DYYTEXT_POINTER=1 -DSTDC_HEADERS=1 -DKERNEL_FILE=\"/kernel\" -D_SMP_SUPPORT=1 -DWITH_STABS_PCIBUS_SET=1 -DWITH_KLD=1 -D_WITHOUT_DDSCP=1 -D_WITHOUT_MONITOR=1 -DWITHOUT_LOOPBACK=1  -I. -I$(srcdir) 
CPPFLAGS = 
LDFLAGS = 
LIBS = 
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
ir_OBJECTS =  autorouting.o building.o config.o erreurs.o global.o \
globals.o misc.o rcube.o reseau.o routage.o yslime.o lslime.o
ir_DEPENDENCIES = 
ir_LDFLAGS = 
LEX_OUTPUT_ROOT = lex.yy
LEXLIB = -lfl
CFLAGS = -g -O2 -Wall
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
DATA =  $(pkgdata_DATA)

DIST_COMMON =  Makefile.am Makefile.in lslime.c yslime.c


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
DEP_FILES =  .deps/autorouting.P .deps/building.P .deps/config.P \
.deps/erreurs.P .deps/global.P .deps/globals.P .deps/lslime.P \
.deps/misc.P .deps/rcube.P .deps/reseau.P .deps/routage.P \
.deps/yslime.P
SOURCES = $(ir_SOURCES)
OBJECTS = $(ir_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .l .o .s .y
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu routing/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


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
.l.c:
	$(LEX) $(AM_LFLAGS) $(LFLAGS) $< && mv $(LEX_OUTPUT_ROOT).c $@
.y.c:
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $< && mv y.tab.c $*.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi
yslime.h: yslime.c


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

subdir = routing

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu routing/Makefile
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
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-binPROGRAMS
install-exec: install-exec-am

install-data-am: install-pkgdataDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-binPROGRAMS uninstall-pkgdataDATA
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(bindir) $(DESTDIR)$(pkgdatadir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
	-test -z "lslimelyslimehyslimec" || rm -f lslimel yslimeh yslimec
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-tags mostlyclean-depend mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-binPROGRAMS clean-compile clean-tags clean-depend \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-tags \
		distclean-depend distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-depend maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile uninstall-pkgdataDATA install-pkgdataDATA tags \
mostlyclean-tags distclean-tags clean-tags maintainer-clean-tags \
distdir mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


ir:	$(ir_OBJECTS)
	$(CC) -o ir autorouting.o building.o config.o erreurs.o global.o \
	globals.o misc.o rcube.o reseau.o routage.o lslime.o -lfl

router_driver.o: router_driver.c
	$(CC) -c router_driver.c

generator_driver.o: generator_driver.c
	$(CC) -c generator_driver.c

moteur.o: moteur.c
	$(CC) -c moteur.c

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
