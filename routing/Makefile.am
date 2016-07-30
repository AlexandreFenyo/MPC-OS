
# $Id: Makefile.am,v 1.1.1.1 1998/10/28 21:07:34 alex Exp $

bin_PROGRAMS = ir

pkgdata_DATA = router_driver.o generator_driver.o moteur.o

ir_SOURCES = autorouting.c building.c config.c erreurs.c global.c \
             globals.c misc.c rcube.c reseau.c routage.c yslime.y lslime.l

ir_LDADD = @LEXLIB@

ir:	$(ir_OBJECTS)
	$(CC) -o ir autorouting.o building.o config.o erreurs.o global.o \
	globals.o misc.o rcube.o reseau.o routage.o lslime.o -lfl

router_driver.o: router_driver.c
	$(CC) -c router_driver.c

generator_driver.o: generator_driver.c
	$(CC) -c generator_driver.c

moteur.o: moteur.c
	$(CC) -c moteur.c

EXTRA_DIST = autorouting.h building.h config.h erreurs.h generic.h globals.h misc.h \
	     reseau.h routage.h yslime.h router_driver.c generator_driver.c moteur.c

