# Makefile for the Replicated FileSystem server and client

CC=	gcc -g

CCFLAGS=	-Wall

all:	server \
	client \
	mote-gw.tgz

mote-gw.tgz:	server client
		tar cvf mote-gw.tar *.c *.h Makefile 
		gzip mote-gw.tar

libmote-gw.a:	libmote-gw.c mote-gw.h
		echo "Making libraries"
		$(CC) -c libmote-gw.c
		ar -rc libmote-gw.a libmote-gw.o
		rm libmote-gw.o

server:		server.c server.h libmote-gw.a uart.c
		echo "Making server"
		$(CC) $(CCFLAGS) -o server server.c uart.c -L. -lmote-gw -lpthread

client:		client.c libmote-gw.a
		echo "Making client"
		$(CC) -o $@ $@.c -L. -lmote-gw -lpthread $(CCFLAGS)

utest:		uart.c utest.c
		$(CC) -o $@ $@.c uart.c -L. -lmote-gw -lpthread $(CCFLAGS)

clean:
	-rm -f *~ server client *.a *.o core *.tar.gz
