
# $Id: Makefile.am,v 1.5 2000/02/23 18:11:25 alex Exp $

bin_PROGRAMS = hslclient hslserver malloc virt2phys readphys writephys \
	       getevents vmconst init_pcibus readpci writepci vmspace \
	       dumpPCIDDC setmode testputrecv testputsend testputbench \
	       dumptables prot_dump prot_wire prot_garbcoll msqsend \
	       socksend demo hsl2tty testscp mmapbench dumpchannelstate \
	       selectnotice testputsend_loop bench_latence bench_throughput \
	       testtimer checkcalibration simulateEEP dumpINTR

bin_SCRIPTS = startdebug.sh startdebug-vmd.sh

lib_LIBRARIES   = libuseraccess.a
include_HEADERS = useraccess.h

libuseraccess_a_SOURCES = useraccess.c

init_pcibus_LDADD = -lkvm

CFLAGS = ${DEFS} -g ${OPTFLAGS} -Wall @DEBUG_FLAGS@ -O2 @SMP_FLAGS_1@

hslclient_SOURCES = hslclient.c rpctransport_clnt.c
rpctransport_clnt.c: rpctransport.x
	@RPCGEN@ $?

hslserver_SOURCES = hslserver.c rpctransport_svc.c
rpctransport_svc.c: rpctransport.x
	@RPCGEN@ $?

BUILT_SOURCES = rpctransport_xdr.h rpctransport.h

rpctransport_xdr.h: rpctransport.x
	@RPCGEN@ -c $? > $@

rpctransport.h: rpctransport.x
	@RPCGEN@ $?

startdebug.sh: startdebug.sh.m4
	if @TEST@ @X_HOSTS@ != no ; \
	then \
	  @M4@ -DHOST1=@HOST1@ -DHOST2=@HOST2@ -DXHOST=@XHOST@ \
	       -DRSH=@RSH@ -DXTERM=@XTERM@ -DHOSTNAME=`@HOSTNAME@` \
	       -DCSH=@CSH@ -DWHERE=@PWD@/modules -DSHELL=@X_SHELL@ \
	       startdebug.sh.m4 > startdebug.sh ; \
	  @CHMOD@ +x startdebug.sh ; \
	fi

startdebug-vmd.sh: startdebug-vmd.sh.m4
	if @TEST@ @X_HOSTS@ != no ; \
	then \
	  @M4@ -DHOST1=@HOST1@ -DHOST2=@HOST2@ -DXHOST=@XHOST@ \
	       -DRSH=@RSH@ -DXTERM=@XTERM@ -DHOSTNAME=`@HOSTNAME@` \
	       -DCSH=@CSH@ -DWHERE=@PWD@/MPC_OS -DSHELL=@X_SHELL@ \
	       startdebug-vmd.sh.m4 > startdebug-vmd.sh ; \
	  @CHMOD@ +x startdebug-vmd.sh ; \
	fi

testputrecv_LDADD = libuseraccess.a
testputrecv: libuseraccess.a

testputsend_LDADD = libuseraccess.a
testputsend: libuseraccess.a

testputsend_loop_LDADD = lubuseraccess.a
testputsend_loop: libuseraccess.a

testputbench_LDADD = libuseraccess.a
testputbench: libuseraccess.a

bench_latence_LDADD = libuseraccess.a
bench_latence: libuseraccess.a

bench_throughput_LDADD = libuseraccess.a
bench_throughput: libuseraccess.a

CLEANFILES = ${BUILT_SOURCES} rpctransport_clnt.c rpctransport_svc.c \
             rpctransport_xdr.c startdebug.sh startdebug-vmd.sh

EXTRA_DIST = hslclient.h hslserver.h rpctransport.x startdebug.sh.m4 \
             startdebug-vmd.sh.m4

# makedepend doesn't like when a header file is included in a source file
# but doesn't exist...
dist-hook:
	-@RM@ $(distdir)/rpctransport.h
#	@TOUCH@ $(distdir)/rpctransport.h
#	sleep 1
#	@TOUCH@ $(distdir)/rpctransport.x # this line to force rpctransport.h
	                                  # to be built

