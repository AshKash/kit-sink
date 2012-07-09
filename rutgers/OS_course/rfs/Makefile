# Makefile for the Replicated FileSystem server and client

CC=	gcc -g

CCFLAGS=	-Wall -lnsl -lsocket

all:	server \
	client \
	rfs.tgz

rfs.tgz:	server \
		controller \
		client
		tar cvf rfs.tar *.c *.h Makefile 
		gzip rfs.tar

controller:	controller.c \
		librfs.a
		echo "Making the controller"
		$(CC) $(CCFLAGS) controller.c -o controller -L. -lrfs -lpthread

librfs.a:	librfs.c
		echo "Making libraries"
		$(CC) -Wall -c librfs.c
		ar -rc librfs.a librfs.o
		rm librfs.o

server:		server.c \
		librfs.a
		echo "Making server"
		$(CC) $(CCFLAGS) -o server server.c -L. -lrfs -lpthread -lnsl -lsocket

client:		client.c \
		librfs.a
		echo "Making client"
		$(CC) $(CCFLAGS) -o client client.c -L. -lrfs

clean:
	rm -f server client controller *.a *.o core rfs.tar.gz
