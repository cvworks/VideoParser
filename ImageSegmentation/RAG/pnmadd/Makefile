LIBSRCS = pnmadd.c xpm.c pseudo.c array.c png.c pnmemd.c colorhist.c \
          label.c
INCLUDES = pnmadd.h xpm.h pseudo.h array.h png.h pnmemd.h colorhist.h \
           label.h
LIBOBJS = $(LIBSRCS:.c=.o)
LIB = libpnmadd.a
BIN = mkEMDmatrix

BINOBJS = $(BIN).o
LDLIBS = -lpnmadd -lemd -llist -lrs -lm

ROOT = ..
LIBDIR = $(ROOT)/lib
INCDIR = $(ROOT)
INCLUDEDIR = .

CFLAGS = -O4 -I$(INCDIR) -I/usr/X11/include 	
#LDFLAGS = -L$(LIBDIR) -L/usr/X11/lib -lppm -lpgm -lpbm -lpng -lz -lm
LDFLAGS = -L$(LIBDIR) -L/usr/lib -lnetpbm -lpng -lz -lm

all: $(LIB) $(BIN)

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

$(BIN): $(BINOBJS) $(LIB)
	$(CC) $(CFLAGS) $(BIN).c -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o $(BIN) $(LIB)

