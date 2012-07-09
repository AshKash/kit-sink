# Makefile for the Replicated FileSystem server and client

CC=	gcc -g

CCFLAGS=	-Wall -lnsl -lsocket

all:	server \
	client \
	perf_test\
	rfs.tgz

rfs.tgz:	server \
		client
		tar cvf rfs.tar *.c *.h Makefile 
		gzip rfs.tar

librfs.a:	librfs.c
		echo "Making libraries"
		$(CC) -c librfs.c
		ar -rc librfs.a librfs.o
		rm librfs.o

server:		server.c \
		librfs.a
		echo "Making server"
		$(CC) $(CCFLAGS) -o server server.c -L. -lrfs -lpthread

client:		client.c \
		librfs.a
		echo "Making client"
		$(CC) -o client client.c -L. -lrfs -lpthread $(CCFLAGS)

perf_test:	perf_test.c \
		librfs.a
		echo "Making perf_test"
		$(CC) -o perf_test perf_test.c -L. -lrfs -lpthread $(CCFLAGS)

clean:
	rm -f server client perf_test *.a *.o core rfs.tar.gz
