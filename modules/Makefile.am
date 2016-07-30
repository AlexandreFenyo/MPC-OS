
# $Id: Makefile.am,v 1.6 2000/02/08 18:54:31 alex Exp $

@SET_MAKE@

MODNAME       = hsldriver
CMEMMODNAME   = cmemdriver
SERIALMODNAME = serialdriver

DEVNAME       = /dev/hsl
CMEMDEVNAME   = /dev/cmem
SERIALDEVNAME = /dev/serial

SERIALMAJOR   = 120
CMEMMAJOR     = 121
DEVMAJOR      = 122

include_HEADERS = data.h ddslrpv.h ddslrpp.h put.h driver.h cmem.h flowcp.h mdcp.h

pkglib_PROGRAMS = hsldriver cmemdriver serialdriver

hsldriver_SOURCES = driver.c put.c ddslrpp.c ddslrpv.c flowcp.c mdcp.c

cmemdriver_SOURCES = cmem.c

serialdriver_SOURCES = serialdriver.c

hsldriver: $(hsldriver_SOURCES:.c=.o) cmemdriver
	if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(GENSETDEFS) mdcp.o flowcp.o ddslrpv.o ddslrpp.o put.o driver.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o hsldriver_mod.ko setdef0.o mdcp.o flowcp.o ddslrpv.o ddslrpp.o put.o driver.o setdef1.o ; \
	fi
	$(LD) -r @SMP_FLAGS_3@ -o hsldriver $(hsldriver_SOURCES:.c=.o)

CFLAGS = -g ${OPTFLAGS} -DKERNEL -Wall @DEBUG_FLAGS@ -O2 -DNO_CONFIG_READ @SMP_FLAGS_1@ @SMP_FLAGS_3@ @WORKAROUND_FLAGS@

cmemdriver: $(cmemdriver_SOURCES:.c=.o)
	$(LD) -r @SMP_FLAGS_3@ -o cmemdriver $(cmemdriver_SOURCES:.c=.o)

cmem.o: cmem.c
	$(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) -g -DKERNEL -Wall \
		@DEBUG_FLAGS@ @SMP_FLAGS_2@ @SMP_FLAGS_3@ -O cmem.c
	if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(GENSETDEFS) cmem.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o cmemdriver_mod.ko setdef0.o cmem.o setdef1.o ; \
	fi

serialdriver: $(serialdriver_SOURCES:.c=.o)
	$(LD) -r @SMP_FLAGS_3@ -o serialdriver $(serialdriver_SOURCES:.c=.o)

serialdriver.o: serialdriver.c
	$(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) -g -DKERNEL -Wall \
		@DEBUG_FLAGS@ @SMP_FLAGS_2@ @SMP_FLAGS_3@ -O serialdriver.c
	if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(GENSETDEFS) serialdriver.o ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef0.c ; \
	$(CC) -O -pipe -DKERNEL -Wreturn-type -Wcomment -Wredundant-decls -Wimplicit -Wnested-externs -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith -Winline -Wuninitialized -Wformat -Wunused  -fformat-extensions -ansi -DKLD_MODULE -nostdinc -I- -I. -I/usr/include -c setdef1.c ; \
	$(LD) -Bshareable -o serialdriver_mod.ko setdef0.o serialdriver.o setdef1.o ; \
	fi

CLEANFILES = hsldriver.sym serialdriver_mod.ko cmemdriver_mod.ko hsldriver_mod.ko setdef0.o setdef0.c setdef1.o setdef1.c setdefs.h

load:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	@-@LN_S@ hsldriver hsldriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	if @TEST@ @WITH_STABS@ = yes ; \
	then \
	  @M4@ @SMP_FLAGS_2@ -D_CLIP_START=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_start$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_CLIP_END=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_end$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -DNSELCOLL=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _nselcoll$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _pcibus_set$$ \
	                    | @CUT@ -f 1 -d ' '` pcibus.S.m4 > pcibus.S ; \
          @AS@ @SMP_FLAGS_3@ -o pcibus.o pcibus.S ; \
	  @LD@ @SMP_FLAGS_3@ -A cmemdriver.sym -o cmemdriver-pcibus.sym pcibus.o ; \
	  @MODLOAD@ -e hslinit -A cmemdriver-pcibus.sym \
	                 -o hsldriver.sym hsldriver.o ; \
	else \
	  @MODLOAD@ -e hslinit -A cmemdriver.sym -o hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	@MODSTAT@ -n $(MODNAME)_mod ; \
	fi
	@$(MAKE) device
	@if @TEST@ @WITH_STABS@ = no ; \
	then \
	  init_pcibus ; \
        fi
	@echo done.

load_@HOST1@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	-@LN_S@ hsldriver hsldriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	if @TEST@ @WITH_STABS@ = yes ; \
	then \
	  @M4@ @SMP_FLAGS_2@ -D_CLIP_START=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_start$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_CLIP_END=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_end$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -DNSELCOLL=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _nselcoll$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _pcibus_set$$ \
	                    | @CUT@ -f 1 -d ' '` \
	                      pcibus.S.m4 > @HOST1@--pcibus.S ; \
          @AS@ @SMP_FLAGS_3@ -o pcibus.o @HOST1@--pcibus.S ; \
	  @LD@ @SMP_FLAGS_3@ -A @HOST1@--cmemdriver.sym -o @HOST1@--cmemdriver-pcibus.sym \
	                                                         pcibus.o ; \
	  @MODLOAD@ -e hslinit -A @HOST1@--cmemdriver-pcibus.sym \
	                 -o @HOST1@--hsldriver.sym hsldriver.o ; \
	else \
	  @MODLOAD@ -e hslinit -A cmemdriver.sym -o @HOST1@--hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	@MODSTAT@ -n $(MODNAME)_mod ; \
	fi
	$(MAKE) device
	if @TEST@ @WITH_STABS@ = no ; \
	then \
	  init_pcibus ; \
        fi
	-@RM@ -f @HOST1@--pcibus.S @HOST1@--cmemdriver-pcibus.sym
	@echo done.

load_@HOST2@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko hsldriver_mod.ko serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDLOAD) -v hsldriver_mod.ko ; \
	$(KLDSTAT) ; \
	fi
	-@LN_S@ hsldriver hsldriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	if @TEST@ @WITH_STABS@ = yes ; \
	then \
	  @M4@ @SMP_FLAGS_2@ -D_CLIP_START=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_start$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_CLIP_END=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _vm_map_clip_end$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -DNSELCOLL=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _nselcoll$$ \
	                    | @CUT@ -f 1 -d ' '`  \
	       -D_PCIBUS_SET=0x`@NM@ @SMP_FLAGS_3@ @KERNEL_FILE@ | @GREP@ _pcibus_set$$ \
	                    | @CUT@ -f 1 -d ' '` \
	                      pcibus.S.m4 > @HOST2@--pcibus.S ; \
          @AS@ @SMP_FLAGS_3@ -o pcibus.o @HOST2@--pcibus.S ; \
	  @LD@ @SMP_FLAGS_3@ -A @HOST2@--cmemdriver.sym -o @HOST2@--cmemdriver-pcibus.sym \
	                                                         pcibus.o ; \
	  @MODLOAD@ -e hslinit -A @HOST2@--cmemdriver-pcibus.sym \
	                 -o @HOST2@--hsldriver.sym hsldriver.o ; \
	else \
	  @MODLOAD@ -e hslinit -A cmemdriver.sym -o @HOST2@--hsldriver.sym \
                                                    hsldriver.o ; \
	fi ; \
	@MODSTAT@ -n $(MODNAME)_mod ; \
	fi
	$(MAKE) device
	if @TEST@ @WITH_STABS@ = no ; \
	then \
	  init_pcibus ; \
        fi
	-@RM@ -f @HOST2@--pcibus.S @HOST2@--cmemdriver-pcibus.sym
	@echo done.

loadcmem:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	@-@LN_S@ cmemdriver cmemdriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODLOAD@ -A @KERNEL_FILE@ -e cmeminit -o cmemdriver.sym cmemdriver.o ; \
	@MODSTAT@ -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadcmem_@HOST1@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	-@LN_S@ cmemdriver cmemdriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODLOAD@ -A @KERNEL_FILE@ -e cmeminit -o @HOST1@--cmemdriver.sym \
	                                                     cmemdriver.o ; \
	@MODSTAT@ -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadcmem_@HOST2@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) cmemdriver_mod.ko /modules ; \
	$(KLDLOAD) -v cmemdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) cmemdevice ; \
	fi
	-@LN_S@ cmemdriver cmemdriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODLOAD@ -A @KERNEL_FILE@ -e cmeminit -o @HOST2@--cmemdriver.sym \
	                                                     cmemdriver.o ; \
	@MODSTAT@ -n $(CMEMMODNAME)_mod ; \
	$(MAKE) cmemdevice ; \
	fi
	echo done.

loadserial:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(CP) serialdriver_mod.ko /modules ; \
	$(KLDLOAD) -v serialdriver_mod.ko ; \
	$(KLDSTAT) ; \
	$(MAKE) serialdevice ; \
	fi
	@-@LN_S@ serialdriver serialdriver.o
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODLOAD@ -e serialinit -o serialdriver.sym serialdriver.o ; \
	@MODSTAT@ -n $(SERIALMODNAME)_mod ; \
	$(MAKE) serialdevice ; \
	fi
	echo done.

unload:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(MODNAME)_mod ; \
	fi
	echo done.

unload_@HOST1@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(MODNAME)_mod ; \
	fi
	echo done.

unload_@HOST2@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n hsldriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(MODNAME)_mod ; \
	fi
	echo done.

unloadcmem:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadcmem_@HOST1@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadcmem_@HOST2@:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n cmemdriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(CMEMMODNAME)_mod ; \
	fi
	echo done.

unloadserial:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	$(KLDUNLOAD) -v -n serialdriver_mod.ko ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MODUNLOAD@ -n $(SERIALMODNAME)_mod ; \
	fi
	echo done.

device:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@@RM@ -f $(DEVNAME)
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	@MKNOD@ $(DEVNAME) c $(DEVMAJOR) 0 ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MKNOD@ $(DEVNAME) c `@MODSTAT@ | @GREP@ $(MODNAME) | \
		@SED@ 's/  */ /g' | @CUT@ -f 3 -d ' '` 0 ; \
	fi
	@CHMOD@ 666 $(DEVNAME) ;
	@@LS@ -l $(DEVNAME)

cmemdevice:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@@RM@ -f $(CMEMDEVNAME)
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	@MKNOD@ $(CMEMDEVNAME) c $(CMEMMAJOR) 0 ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MKNOD@ $(CMEMDEVNAME) c `@MODSTAT@ | @GREP@ $(CMEMMODNAME) | \
		@SED@ 's/  */ /g' | @CUT@ -f 3 -d ' '` 0 ; \
	fi
	@@CHMOD@ 666 $(CMEMDEVNAME)
	@@LS@ -l $(CMEMDEVNAME)

serialdevice:
	@if @TEST@ `@ID@ -u` -ne 0 ; then echo Must be ROOT ; exit 1 ; fi
	@@RM@ -f $(SERIALDEVNAME)
	@if @TEST@ @WITH_KLD@ = yes ; \
	then \
	@MKNOD@ $(SERIALDEVNAME) c $(SERIALMAJOR) 0 ; \
	exit 0 ; \
	fi
	@if @TEST@ @WITH_KLD@ != yes ; \
	then \
	@MKNOD@ $(SERIALDEVNAME) c `@MODSTAT@ | @GREP@ $(SERIALMODNAME) | \
		@SED@ 's/  */ /g' | @CUT@ -f 3 -d ' '` 0 ; \
	fi
	@@CHMOD@ 666 $(SERIALDEVNAME)
	@@LS@ -l $(SERIALDEVNAME)

client_@HOST1@:
	../src/hslclient -f ../config_mpc/hsl-@HOST1@.conf -d /dev/hsl -r ../config_mpc/net-2-res-@HOST1@.raw

client_@HOST2@:
	../src/hslclient -f ../config_mpc/hsl-@HOST2@.conf -d /dev/hsl -r ../config_mpc/net-2-res-@HOST2@.raw

server_@HOST1@:
	../src/hslserver

server_@HOST2@:
	../src/hslserver

EXTRA_DIST = .gdbinit pcibus.S.m4

