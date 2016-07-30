
# $Id: Makefile.am,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $

pkgdata_DATA = hsl-@HOST1@.conf hsl-@HOST2@.conf

hsl-@HOST1@.conf: hsl.conf.m4
	-if @TEST@ X@X_HOSTS@ = Xno ; \
	then                          \
	  exit 1 ;                    \
	fi
	@M4@ -DNODE1=0 -DNODE2=1 -DHOSTNAME2=@HOST2@ \
	              hsl.conf.m4 > hsl-@HOST1@.conf

hsl-@HOST2@.conf: hsl.conf.m4
	-if @TEST@ X@X_HOSTS@ = Xno ; \
	then                          \
	  exit 1 ;                    \
	fi
	@M4@ -DNODE1=1 -DNODE2=0 -DHOSTNAME2=@HOST1@ \
	              hsl.conf.m4 > hsl-@HOST2@.conf

CLEANFILES = hsl-@HOST1@.conf hsl-@HOST2@.conf

EXTRA_DIST = hsl.conf.m4 types.cfg Catalog config.ml generator.mmd router.mmd net-1-res.mle net-1.mle net-2-res.mle net-2.mle net-3-res.mle net-3.mle net-4-res.mle net-4.mle net-1-loopback-res.mle net-1-loopback.mle net-1-loopback-res.raw 1host.cluster example.cluster

