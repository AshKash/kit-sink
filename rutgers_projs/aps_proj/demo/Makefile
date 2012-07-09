# Makefile for the Replicated FileSystem server and client

GROUP= 0x13
CC=	gcc -g

CCFLAGS=	-Wall -DLOCAL_GROUP=$(GROUP) -D"TOS_EVENT(event_name)"=event_name##_EVENT -pedantic

all:    linkx \
	server \
	client \
	aps-query \
	mote-gw.tgz

mote-gw.tgz:	server client
		tar cvf mote-gw.tar *.c *.h Makefile 
		gzip mote-gw.tar

libmote-gw.a:	libmote-gw.c mote-gw.h
		echo "Making libraries"
		$(CC) $(CCFLAGS) -c libmote-gw.c
		ar -rc libmote-gw.a libmote-gw.o
		rm libmote-gw.o

server:		server.c server.h libmote-gw.a uart.c
		echo "Making server"
		$(CC) $(CCFLAGS) -o server server.c uart.c -L. -lmote-gw -lpthread

client:		client.c libmote-gw.a
		echo "Making client"
		$(CC) -o $@ $@.c -L. -lmote-gw -lpthread $(CCFLAGS)

aps-query:	aps-query.c libmote-gw.a ../system/fixpoint.c
		echo "Making aps-query"
		$(CC) -o $@ $@.c ../system/fixpoint.c -I ../system/include -DFULLPC=1 -L. -lmote-gw -lpthread -lm $(CCFLAGS)

utest:		uart.c utest.c
		$(CC) -o $@ $@.c uart.c -L. -lmote-gw -lpthread $(CCFLAGS)

linkx:		
		ln -sf ../system/include/MSG.h
clean:
	-rm -f *~ server client *.a *.o core *.tar.gz aps-query *.tar
