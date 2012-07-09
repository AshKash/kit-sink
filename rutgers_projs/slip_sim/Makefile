# Makefile to make the goodies

CC=	gcc
CCFLAGS=	-g -Wall

all:	tty_server port_fwd rfm_server cat_null

tty_server:	tty_server.c
		$(CC) $(CCFLAGS) -o $@ $@.c -lutil

cat_null:	cat_null.c
		$(CC) -fprofile-arcs -ftest-coverage  $(CCFLAGS) -o $@ $@.c -lutil

port_fwd:	port_fwd.c
		$(CC) $(CCFLAGS) -o $@ $@.c

rfm_server:	rfm_server.c
		$(CC) $(CCFLAGS) -o $@ $@.c

clean:		
		-rm -f *~ *.a *.o core tty_server port_fwd rfm_server

