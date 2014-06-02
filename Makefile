CC=			gcc
CFLAGS=		-g -Wall -O2
DFLAGS=
PROG=		crlf
INCLUDES=	
LIBS=		-lz -lpthread

.SUFFIXES:.c .o

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

crlf:crlf.o recode.o
		$(CC) $(CFLAGS) $(DFLAGS) $^ -o $@ $(LIBS)

crlf.o:crlf.h
recode.o:crlf.h

clean:
		rm -fr gmon.out *.o ext/*.o a.out $(PROG) *~ *.a *.dSYM session*
