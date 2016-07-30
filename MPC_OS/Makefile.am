
# $Id: Makefile.am,v 1.3 2000/03/08 21:45:22 alex Exp $

# IT WORKS (for gcc) :
# CFLAGS += foo

# -D_THREAD_SAFE pour redefinir une macro a la place de errno

# CXXFLAGS must NOT contain -O2 (it would generate an internal gcc error)
CXXFLAGS = -g @SMP_FLAGS_1@ @SMP_FLAGS_2@
CFLAGS += -g @SMP_FLAGS_1@ @SMP_FLAGS_2@

SUFFIXES = .cc
bin_PROGRAMS = vmd testlibmpc testlibmpc2 testlibmpc3 \
               testlibmpc_read testlibmpc_write testlibmpc_select

vmd_SOURCES = vmd.cc distobj.cc vmspace.cc object.cc typeid.cc

testlibmpc_LDADD  = libmpc.a
testlibmpc2_LDADD = libmpc.a
testlibmpc3_LDADD = libmpc.a
testlibmpc_read_LDADD   = libmpc.a
testlibmpc_write_LDADD  = libmpc.a
testlibmpc_select_LDADD = libmpc.a

lib_LIBRARIES   = libmpc.a
include_HEADERS = mpc.h mpcshare.h

libmpc_a_SOURCES = mpc.c

# Don't want to use libtool...
pkgdata_DATA = libsocketwrap.so.1.0 libmpc.so.1.0

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
	if @TEST@ "X@SMP_FLAGS_2@Y" = XY ; \
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
	if @TEST@ "X@SMP_FLAGS_2@Y" = XY ; \
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
	if @TEST@ "X@SMP_FLAGS_2@Y" = XY ; \
	then \
	  $(LD) -Bshareable -o libmpc.so.1.0 /usr/lib/c++rt0.o mpc-pic.o ; \
	else \
	  $(LD) -Bshareable -o libmpc.so.1.0 mpc-pic.o ; \
	fi

socketwrap.o: socketwrap.c
	$(CC) -fPIC @DEBUG_FLAGS@ $(CFLAGS) -c socketwrap.c

mpc-pic.o: mpc.c mpc.h mpcshare.h
	$(CC) -fPIC @DEBUG_FLAGS@ $(CFLAGS) -o mpc-pic.o -c mpc.c

EXTRA_DIST = transcript.pl vmd.h distobj.h distobj.dist.h object.h \
             typeid.h vmspace.h socketwrap.c stopvmd.sh loader.sh

CLEANFILES = libmpc.so.1.0 libsocketwrap.so.1.0 distobj.h
