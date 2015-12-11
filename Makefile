#
# Makefile del sistema de ficheros
#

INCLUDEDIR=include
CC=gcc
CFLAGS=-g -Wall -I$(INCLUDEDIR)
AR=ar
MAKE=make

OBJS_DEV=filesystem.o
LIB=libfs.a


all: $(LIB)
	$(CC) $(CFLAGS) -o test test.c libfs.a

filesystem.o: $(INCLUDEDIR)/filesystem.h

$(LIB): $(OBJS_DEV)
	$(AR) rcv $@ $^

clean:
	rm -f $(LIB) $(OBJS_DEV) test
