APP_INCLUDE = -I ./include
APP_OBJS= algo.o stack.o
DESC=tbf.desc

ADDR=10
LOCX=15.0
LOCY=15.0

TBF_DEFS=-DLOCX=$(LOCX) -DLOCY=$(LOCY) -Iinclude/

APP_LIBS = -lm
SHARED_LOCAL = shared/

bintest:	mapping.h test.c stack.c algo.c intopost.c include/tbf_hdr.h
		gcc -g -Wall -lm algo.c test.c stack.c intopost.c -o bintest $(TBF_DEFS) -DTBF_DEBUG

mapping.h:	mapping.txt
		./genhdr.pl $< > ./include/mapping.h

algo.o:		algo.c mapping.h include/tbf_hdr.h
		$(CC) -Wall -c algo.c $(TBF_DEFS)

stack.o:		stack.c mapping.h include/tbf_hdr.h
		$(CC) -Wall -c stack.c $(TBF_DEFS)

include ../Makeinclude
