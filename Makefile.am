
# $Id: Makefile.am,v 1.2 1999/02/13 15:11:58 alex Exp $

@SET_MAKE@

SUBDIRS = src modules examples config_mpc MPC_OS mpcview routing # docs

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
	@WC@ -l Makefile.am Makefile.in configure.in \
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
