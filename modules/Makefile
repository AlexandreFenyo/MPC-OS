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

# $Id: Makefile.am,v 1.6 2000/02/08 18:54:31 alex Exp $


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

MODNAME = hsldriver
CMEMMODNAME = cmemdriver
SERIALMODNAME = serialdriver

DEVNAME = /dev/hsl
CMEMDEVNAME = /dev/cmem
SERIALDEVNAME = /dev/serial

SERIALMAJOR = 120
CMEMMAJOR = 121
DEVMAJOR = 122

include_HEADERS = data.h ddslrpv.h ddslrpp.h put.h driver.h cmem.h flowcp.h mdcp.h

pkglib_PROGRAMS = hsldriver cmemdriver serialdriver

hsldriver_SOURCES = driver.c put.c ddslrpp.c ddslrpv.c flowcp.c mdcp.c

cmemdriver_SOURCES = cmem.c

serialdriver_SOURCES = serialdriver.c

CFLAGS = -g ${OPTFLAGS} -DKERNEL -Wall  -DMPC_STATS -O2 -DNO_CONFIG_READ -I/usr/include -I/sys -elf -DPCIDDC2ndRun

CLEANFILES = hsldriver.sym serialdriver_mod.ko cmemdriver_mod.ko hsldriver_mod.ko setdef0.o setdef0.c setdef1.o setdef1.c setdefs.h

EXTRA_DIST = .gdbinit pcibus.S.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_CLEAN_FILES = 
PROGRAMS =  $(pkglib_PROGRAMS)


DEFS =  -DYYTEXT_POINTER=1 -DSTDC_HEADERS=1 -DKERNEL_FILE=\"/kernel\" -D_SMP_SUPPORT=1 -DWITH_STABS_PCIBUS_SET=1 -DWITH_KLD=1 -D_WITHOUT_DDSCP=1 -D_WITHOUT_MONITOR=1 -DWITHOUT_LOOPBACK=1  -I. -I$(srcdir) 
CPPFLAGS = 
LDFLAGS = 
LIBS = 
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
hsldriver_OBJECTS =  driver.o put.o ddslrpp.o ddslrpv.o flowcp.o mdcp.o
hsldriver_LDADD = $(LDADD)
hsldriver_DEPENDENCIES = 
hsldriver_LDFLAGS = 
cmemdriver_OBJECTS =  cmem.o
cmemdriver_LDADD = $(LDADD)
cmemdriver_DEPENDENCIES = 
cmemdriver_LDFLAGS = 
serialdriver_OBJECTS =  serialdriver.o
serialdriver_LDADD = $(LDADD)
serialdriver_DEPENDENCIES = 
serialdriver_LDFLAGS = 
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(include_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
DEP_FILES =  .deps/cmem.P .deps/ddslrpp.P .deps/ddslrpv.P .deps/driver.P \
.deps/flowcp.P .deps/mdcp.P .deps/put.P .deps/serialdriver.P
SOURCES = $(hsldriver_SOURCES) $(cmemdriver_SOURCES) $(serialdriver_SOURCES)
OBJECTS = $(hsldriver_OBJECTS) $(cmemdriver_OBJECTS) $(serialdriver_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu modules/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-pkglibPROGRAMS:

clean-pkglibPROGRAMS:
	-test -z "$(pkglib_PROGRAMS)" || rm -f $(pkglib_PROGRAMS)

distclean-pkglibPROGRAMS:

maintainer-clean-pkglibPROGRAMS:

install-pkglibPROGRAMS: $(pkglib_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkglibdir)
	@list='$(pkglib_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "  $(INSTALL_PROGRAM) $$p $(DESTDIR)$(pkglibdir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	     $(INSTALL_PROGRAM) $$p $(DESTDIR)$(pkglibdir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-pkglibPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(pkglib_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(pkglibdir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
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

subdir = modules

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu modules/Makefile
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
install-exec-am: install-pkglibPROGRAMS
install-exec: install-exec-am

install-data-am: install-includeHEADERS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-pkglibPROGRAMS uninstall-includeHEADERS
uninstall: uninstall-am
all-am: Makefile $(PROGRAMS) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(pkglibdir) $(DESTDIR)$(includedir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-pkglibPROGRAMS mostlyclean-compile \
		mostlyclean-tags mostlyclean-depend mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-pkglibPROGRAMS clean-compile clean-tags clean-depend \
		clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-pkglibPROGRAMS distclean-compile distclean-tags \
		distclean-depend distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-pkglibPROGRAMS \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-depend maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-pkglibPROGRAMS distclean-pkglibPROGRAMS \
clean-pkglibPROGRAMS maintainer-clean-pkglibPROGRAMS \
uninstall-pkglibPROGRAMS install-pkglibPROGRAMS mostlyclean-compile \
distclean-compile clean-compile maintainer-clean-compile \
uninstall-includeHEADERS install-includeHEADERS tags mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir \
mostlyclean-depend distclean-depend clean-depend \
maintainer-clean-depend info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean




hsldriver: $(hsldriver_SOURCES:.c=.o) cmemdriver
	if /bin/test yes = yes ; \
	then \
	$(GENSETDEFS) mdcp.o flowcp.o ddslrpv.o ddslrpp.o put.o driver.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o hsldriver_mod.ko setdef0.o mdcp.o flowcp.o ddslrpv.o ddslrpp.o put.o driver.o setdef1.o ; \
	fi
	$(LD) -r -elf -o hsldriver $(hsldriver_SOURCES:.c=.o)

cmemdriver: $(cmemdriver_SOURCES:.c=.o)
	$(LD) -r -elf -o cmemdriver $(cmemdriver_SOURCES:.c=.o)

cmem.o: cmem.c
	$(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) -g -DKERNEL -Wall \
		 -DMPC_STATS -D_SMP_SUPPORT -elf -O cmem.c
	if /bin/test yes = yes ; \
	then \
	$(GENSETDEFS) cmem.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o cmemdriver_mod.ko setdef0.o cmem.o setdef1.o ; \
	fi

serialdriver: $(serialdriver_SOURCES:.c=.o)
	$(LD) -r -elf -o serialdriver $(serialdriver_SOURCES:.c=.o)

serialdriver.o: serialdriver.c
	$(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) -g -DKERNEL -Wall \
		 -DMPC_STATS -D_SMP_SUPPORT -elf -O serialdriver.c
	if /bin/test yes = yes ; \
	then \
	$(GENSETDEFS) serialdriver.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o serialdriver_mod.ko setdef0.o serialdriver.o setdef1.o ; \
	fi

load:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	@-ln -s hsldriver hsldriver.o
	@if /bin/test yes != yes ; \
	then \
	if /bin/test yes = yes ; \
	then \
	  /usr/bin/m4 -D_SMP_SUPPORT -D_CLIP_START=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_start$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_CLIP_END=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_end$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -DNSELCOLL=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _nselcoll$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _pcibus_set$$ \
	                    | /usr/bin/cut -f 1 -d ' '` pcibus.S.m4 > pcibus.S ; \
          /usr/bin/as -elf -o pcibus.o pcibus.S ; \
	  /usr/bin/ld -elf -A cmemdriver.sym -o cmemdriver-pcibus.sym pcibus.o ; \
	  /sbin/modload -e hslinit -A cmemdriver-pcibus.sym \
	                 -o hsldriver.sym hsldriver.o ; \
	else \
	  /sbin/modload -e hslinit -A cmemdriver.sym -o hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	/usr/bin/modstat -n $(MODNAME)_mod ; \
	fi
	@$(MAKE) device
	@if /bin/test yes = no ; \
	then \
	  init_pcibus ; \
        fi
	@echo done.

load_bench1:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	-ln -s hsldriver hsldriver.o
	@if /bin/test yes != yes ; \
	then \
	if /bin/test yes = yes ; \
	then \
	  /usr/bin/m4 -D_SMP_SUPPORT -D_CLIP_START=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_start$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_CLIP_END=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_end$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -DNSELCOLL=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _nselcoll$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _pcibus_set$$ \
	                    | /usr/bin/cut -f 1 -d ' '` \
	                      pcibus.S.m4 > bench1--pcibus.S ; \
          /usr/bin/as -elf -o pcibus.o bench1--pcibus.S ; \
	  /usr/bin/ld -elf -A bench1--cmemdriver.sym -o bench1--cmemdriver-pcibus.sym \
	                                                         pcibus.o ; \
	  /sbin/modload -e hslinit -A bench1--cmemdriver-pcibus.sym \
	                 -o bench1--hsldriver.sym hsldriver.o ; \
	else \
	  /sbin/modload -e hslinit -A cmemdriver.sym -o bench1--hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	/usr/bin/modstat -n $(MODNAME)_mod ; \
	fi
	$(MAKE) device
	if /bin/test yes = no ; \
	then \
	  init_pcibus ; \
        fi
	-/bin/rm -f bench1--pcibus.S bench1--cmemdriver-pcibus.sym
	@echo done.

load_bench2:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	-ln -s hsldriver hsldriver.o
	@if /bin/test yes != yes ; \
	then \
	if /bin/test yes = yes ; \
	then \
	  /usr/bin/m4 -D_SMP_SUPPORT -D_CLIP_START=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_start$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_CLIP_END=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _vm_map_clip_end$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -DNSELCOLL=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _nselcoll$$ \
	                    | /usr/bin/cut -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`/usr/bin/nm -elf /kernel | /usr/bin/grep _pcibus_set$$ \
	                    | /usr/bin/cut -f 1 -d ' '` \
	                      pcibus.S.m4 > bench2--pcibus.S ; \
          /usr/bin/as -elf -o pcibus.o bench2--pcibus.S ; \
	  /usr/bin/ld -elf -A bench2--cmemdriver.sym -o bench2--cmemdriver-pcibus.sym \
	                                                         pcibus.o ; \
	  /sbin/modload -e hslinit -A bench2--cmemdriver-pcibus.sym \
	                 -o bench2--hsldriver.sym hsldriver.o ; \
	else \
	  /sbin/modload -e hslinit -A cmemdriver.sym -o bench2--hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	/usr/bin/modstat -n $(MODNAME)_mod ; \
	fi
	$(MAKE) device
	if /bin/test yes = no ; \
	then \
	  init_pcibus ; \
        fi
	-/bin/rm -f bench2--pcibus.S bench2--cmemdriver-pcibus.sym
	@echo done.

loadcmem:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	@-ln -s cmemdriver cmemdriver.o
	@if /bin/test yes != yes ; \
	then \
	/sbin/modload -A /kernel -e cmeminit -o cmemdriver.sym cmemdriver.o ; \
	/usr/bin/modstat -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadcmem_bench1:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	-ln -s cmemdriver cmemdriver.o
	@if /bin/test yes != yes ; \
	then \
	/sbin/modload -A /kernel -e cmeminit -o bench1--cmemdriver.sym \
	                                                     cmemdriver.o ; \
	/usr/bin/modstat -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadcmem_bench2:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	-ln -s cmemdriver cmemdriver.o
	@if /bin/test yes != yes ; \
	then \
	/sbin/modload -A /kernel -e cmeminit -o bench2--cmemdriver.sym \
	                                                     cmemdriver.o ; \
	/usr/bin/modstat -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadserial:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(CP) serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v serialdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) serialdevice ; \
	fi
	@-ln -s serialdriver serialdriver.o
	@if /bin/test yes != yes ; \
	then \
	/sbin/modload -e serialinit -o serialdriver.sym serialdriver.o ; \
	/usr/bin/modstat -n $(SERIALMODNAME)_mod ; \
	$(MAKE) serialdevice ; \
	fi
	echo done.

unload:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(MODNAME)_mod ; \
	fi
	echo done.

unload_bench1:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(MODNAME)_mod ; \
	fi
	echo done.

unload_bench2:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(MODNAME)_mod ; \
	fi
	echo done.

unloadcmem:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadcmem_bench1:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadcmem_bench2:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadserial:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if /bin/test yes = yes ; \
	then \
	$(KLDUNLOAD) -v -n serialdriver_mod.ko ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/modunload -n $(SERIALMODNAME)_mod ; \
	fi
	echo done.

device:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@/bin/rm -f $(DEVNAME)
	@if /bin/test yes = yes ; \
	then \
	/sbin/mknod $(DEVNAME) c $(DEVMAJOR) 0 ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/mknod $(DEVNAME) c `/usr/bin/modstat | /usr/bin/grep $(MODNAME) | \
		/usr/bin/sed 's/  */ /g' | /usr/bin/cut -f 3 -d ' '` 0 ; \
	fi
	/bin/chmod 666 $(DEVNAME) ;
	@/bin/ls -l $(DEVNAME)

cmemdevice:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@/bin/rm -f $(CMEMDEVNAME)
	@if /bin/test yes = yes ; \
	then \
	/sbin/mknod $(CMEMDEVNAME) c $(CMEMMAJOR) 0 ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/mknod $(CMEMDEVNAME) c `/usr/bin/modstat | /usr/bin/grep $(CMEMMODNAME) | \
		/usr/bin/sed 's/  */ /g' | /usr/bin/cut -f 3 -d ' '` 0 ; \
	fi
	@/bin/chmod 666 $(CMEMDEVNAME)
	@/bin/ls -l $(CMEMDEVNAME)

serialdevice:
	@if /bin/test `/usr/bin/id -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@/bin/rm -f $(SERIALDEVNAME)
	@if /bin/test yes = yes ; \
	then \
	/sbin/mknod $(SERIALDEVNAME) c $(SERIALMAJOR) 0 ; \
	exit 0 ; \
	fi
	@if /bin/test yes != yes ; \
	then \
	/sbin/mknod $(SERIALDEVNAME) c `/usr/bin/modstat | /usr/bin/grep $(SERIALMODNAME) | \
		/usr/bin/sed 's/  */ /g' | /usr/bin/cut -f 3 -d ' '` 0 ; \
	fi
	@/bin/chmod 666 $(SERIALDEVNAME)
	@/bin/ls -l $(SERIALDEVNAME)

client_bench1:
	../src/hslclient -f ../config_mpc/hsl-bench1.conf -d /dev/hsl -r ../config_mpc/net-2-res-bench1.raw

client_bench2:
	../src/hslclient -f ../config_mpc/hsl-bench2.conf -d /dev/hsl -r ../config_mpc/net-2-res-bench2.raw

server_bench1:
	../src/hslserver

server_bench2:
	../src/hslserver

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
