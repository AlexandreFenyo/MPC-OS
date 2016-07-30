
# $Id: hsl.conf.m4,v 1.2 1999/06/10 17:28:49 alex Exp $

# local node format (in case of "--without-loopback" at configure time):
# node NODE_NUMBER
#
# local node format (in case of "--with-loopback" at configure time):
# node NODE_NUMBER loopback program PROGRAM_NUMBER version VERSION

node NODE1

# node format :
# node NODE_NUMBER hostname HOSTNAME program PROGRAM_NUMBER version VERSION

node NODE2 hostname HOSTNAME2 program 101000 version 2

# EOF
