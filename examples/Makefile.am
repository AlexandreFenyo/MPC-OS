
# $Id: Makefile.am,v 1.3 2000/02/07 18:52:00 alex Exp $

noinst_PROGRAMS = addlpe receivelpe testsend testrecv testsend2 \
                  testrecv2 testsendv testrecvv mmapdev testuseraccess \
                  testsend2wait testrecv2wait vmtest testsendv-prot \
		  testrecvv-prot testselectnotice testsendv2 testrecvv2 \
		  testlatenceslr testpvmslr

testuseraccess_LDADD = ../src/libuseraccess.a
testuseraccess: ../src/libuseraccess.a

CFLAGS += -g @SMP_FLAGS_1@ @SMP_FLAGS_2@

