LIBSRCS = __program.c \
          hashq.c io.c memory.c messages.c name.c
INCLUDES = hashq.h io.h memory.h messages.h name.h
LIBOBJS = $(LIBSRCS:.c=.o)
LIB = librs.a

LIBDIR = ../lib
INCLUDEDIR = ../

CLEAN = $(LIBOBJS)
REMOVE = $(LIB)

CC = gcc

CFLAGS += -O4

all: $(LIB)

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

clean:
	rm -f *.o $(LIB)

