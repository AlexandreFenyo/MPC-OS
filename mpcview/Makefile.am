
# $Id: Makefile.am,v 1.2 1999/01/09 18:58:48 alex Exp $

# NOTE THAT YOU MUST GET THE ELF VERSION OF XFORMS
# WHEN USING FreeBSD 3.0 or ABOVE :
# `wget http://bragg.phys.uwm.edu/~zhao/ftp/bxform-freebsd-elf.tgz'

bin_PROGRAMS = mpcview

mpcview_SOURCES = mpcview.c mpcview_main.c

mpcview_LDADD = @X_LIBS@ @LIBXFORMS@ -lXpm -lX11 -lm

INCLUDES += @X_CFLAGS@ -I/usr/local/include -I/sys

EXTRA_DIST = mpcview.h defil.bitmap logo.xpm

