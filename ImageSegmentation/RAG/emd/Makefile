C_SRCS = libemd.c  normalizedEMD.c
HEADERS = $(wildcard *.h)
INCLUDES = $(HEADERS)

LIBOBJS = $(C_SRCS:%.c=%.o)

LIB = libemd.a
BIN = emd-main

LIBDIR = ../lib
INCLUDEDIR = .

CC = gcc
CFLAGS += -O4

all: $(LIB) $(BIN)

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

$(BIN): $(LIBOBJS) $(BIN).o
	$(CC) -o $@ $(LIBOBJS) $(BIN).o -lm

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

clean:
	rm -f *.o $(BIN) $(LIB)


