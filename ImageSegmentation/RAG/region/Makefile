LIBSRCS = regionlab.c regioninfo.c adjacency.c regionmerge.c \
          rag_mem.c _mem.c rag_gen.c polygon.c
INCLUDES = regionlab.h regioninfo.h adjacency.h regionmerge.h \
           rag_mem.h rag_gen.h polygon.h
LIBOBJS = $(LIBSRCS:.c=.o)
LIB = libregion.a
BIN = testRegion

ROOT = ..
LIBDIR = $(ROOT)/lib
INCDIR = $(ROOT)
INCLUDEDIR = $(ROOT)/region

CC = gcc
CFLAGS += -I$(INCDIR) -I.. -I/usr/X11/include
CFLAGS += -O4

all: $(LIB) $(BIN)

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

$(BIN): $(BIN).o $(LIB)
	$(CC) $(CFLAGS) -o $@ -L$(LIBDIR) $^ -lpnmadd -lm

testContour: testContour.o $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ -L$(LIBDIR) -lpnmadd -lm

install: $(LIB)
	$(INSTALL-dir) $(LIBDIR)
	$(INSTALL-file) $(LIB) $(LIBDIR)
	$(INSTALL-dir) $(INCLUDEDIR)
	$(INSTALL-file) $(INCLUDES) $(INCLUDEDIR)
	touch install

clean:
	rm -f *.o $(BIN) $(LIB)