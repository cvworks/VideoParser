LIBSRCS = int_list.c ulint_list.c float_list.c double_list.c \
          char_list.c pt_list.c \
          int_tuple.c float_tuple.c pt_tuple.c long_tuple.c \
          pairFloat_list.c pairInt_list.c pairLong_list.c \
          tripleInt_list.c \
          double_btree.c int_btree.c
INCL_LS_GEN = lsInt.h lsULInt.h lsFloat.h lsDouble.h lsPt.h lsChar.h \
              lsPairFloat.h lsPairInt.h lsPairLong.h lsTripleInt.h
INCL_TPL_GEN = tupleInt.h tupleFloat.h tupleDouble.h tuplePt.h tupleLong.h
INCL_BTREE_GEN = btreeDouble.h btreeInt.h

INCL_GEN = $(INCL_LS_GEN) $(INCL_TPL_GEN) $(INCL_BTREE_GEN)
LSED_GEN = int-lsed ulint-lsed float-lsed double-lsed char-lsed pt-lsed \
           doubleTpl-lsed pairInt-lsed pairLong-lsed tripleInt-lsed \
           pairFloat-lsed intTpl-lsed floatTpl-lsed ptTpl-lsed \
           longTpl-lsed btreeDouble-lsed btreeInt-lsed
INCLUDES = $(INCL_GEN) list.h list_macro.h tuple.h tuple_macro.h \
           btree.h btree_macro.h \
           iterate.h string+.h buffer.h partition.h random.h \
           string_dict.h hashmap.h
LIBOBJS = $(LIBSRCS:.c=.o) iterate.o string+.o buffer.o \
          partition.o random.o string_dict.o hashmap.o
LIB = liblist.a

LIBDIR = ../lib
INCLUDEDIR = ../

CFLAGS += -O4 -I. -I../
CC = gcc

all: list.h tuple.h btree.h $(LIB)

int-lsed: list-lsed.mk
	./list-lsed.mk lsInt int > int-lsed
	
int_list.c: $(INCL_GEN) _list.c int-lsed
	sed -f int-lsed _list.c > $@
	
lsInt.h: _list.h int-lsed
	sed -f int-lsed _list.h > $@

ulint-lsed: list-lsed.mk
	./list-lsed.mk lsULInt "unsigned long int" > ulint-lsed
	
ulint_list.c: $(INCL_GEN) _list.c ulint-lsed
	sed -f ulint-lsed _list.c > $@
	
lsULInt.h: _list.h ulint-lsed
	sed -f ulint-lsed _list.h > $@

float-lsed: list-lsed.mk
	./list-lsed.mk lsFloat float > float-lsed
	
float_list.c: $(INCL_GEN) _list.c float-lsed
	sed -f float-lsed _list.c > $@
	
lsFloat.h: _list.h float-lsed
	sed -f float-lsed _list.h > $@

double-lsed: list-lsed.mk
	./list-lsed.mk lsDouble double > double-lsed
	
double_list.c: $(INCL_GEN) _list.c double-lsed
	sed -f double-lsed _list.c > $@
	
lsDouble.h: _list.h double-lsed
	sed -f double-lsed _list.h > $@

char-lsed: list-lsed.mk
	./list-lsed.mk lsChar char > char-lsed
	
char_list.c: $(INCL_GEN) _list.c char-lsed
	sed -f char-lsed _list.c > $@
	
lsChar.h: _list.h char-lsed
	sed -f char-lsed _list.h > $@

pt-lsed: list-lsed.mk
	./list-lsed.mk lsPt "void *" > $@
	
pt_list.c: $(INCL_GEN) _list.c pt-lsed
	sed -f pt-lsed _list.c > $@
	
lsPt.h:	_list.h pt-lsed
	sed -f pt-lsed _list.h > $@

list.h: $(INCL_LS_GEN) list.h.mk
	./list.h.mk $(INCL_LS_GEN) > $@

pairFloat-lsed:	list-lsed.mk
	./list-lsed.mk lsPairFloat "pairFloat_t" > $@
	
pairFloat_list.c: $(INCL_GEN) _list.c pairFloat-lsed
	sed -f pairFloat-lsed _list.c > $@
	
lsPairFloat.h: _list.h pairFloat-lsed
	sed -f pairFloat-lsed _list.h > $@

pairInt-lsed: list-lsed.mk
	./list-lsed.mk lsPairInt "pairInt_t" > $@
	
pairInt_list.c: $(INCL_GEN) _list.c pairInt-lsed
	sed -f pairInt-lsed _list.c > $@
	
lsPairInt.h: _list.h pairInt-lsed
	sed -f pairInt-lsed _list.h > $@

pairLong-lsed: list-lsed.mk
	./list-lsed.mk lsPairLong "pairLong_t" > $@
	
pairLong_list.c: $(INCL_GEN) _list.c pairLong-lsed
	sed -f pairLong-lsed _list.c > $@
	
lsPairLong.h: _list.h pairLong-lsed
	sed -f pairLong-lsed _list.h > $@

tripleInt-lsed: list-lsed.mk
	./list-lsed.mk lsTripleInt "tripleInt_t" > $@
	
tripleInt_list.c: $(INCL_GEN) _list.c tripleInt-lsed
	sed -f tripleInt-lsed _list.c > $@
	
lsTripleInt.h: _list.h tripleInt-lsed
	sed -f tripleInt-lsed _list.h > $@

iterate.o: list.h

intTpl-lsed: tuple-lsed.mk
	./tuple-lsed.mk Int "int" > $@
	
int_tuple.c: $(INCL_GEN) _tuple.h intTpl-lsed
	sed -f intTpl-lsed _tuple.c > $@
	
tupleInt.h: _tuple.h intTpl-lsed
	sed -f intTpl-lsed _tuple.h > $@

floatTpl-lsed: tuple-lsed.mk
	./tuple-lsed.mk Float "float" > $@
	
float_tuple.c: $(INCL_GEN) _tuple.h floatTpl-lsed
	sed -f floatTpl-lsed _tuple.c > $@
	
tupleFloat.h: _tuple.h floatTpl-lsed
	sed -f floatTpl-lsed _tuple.h > $@

doubleTpl-lsed: tuple-lsed.mk
	./tuple-lsed.mk Double "double" > $@
	
double_tuple.c: $(INCL_GEN) _tuple.h doubleTpl-lsed
	sed -f doubleTpl-lsed _tuple.c > $@
	
tupleDouble.h: _tuple.h doubleTpl-lsed
	sed -f doubleTpl-lsed _tuple.h > $@

ptTpl-lsed: tuple-lsed.mk
	./tuple-lsed.mk Pt "void *" > $@
	
pt_tuple.c: $(INCL_GEN) _tuple.h ptTpl-lsed
	sed -f ptTpl-lsed _tuple.c > $@
	
tuplePt.h: _tuple.h ptTpl-lsed
	sed -f ptTpl-lsed _tuple.h > $@

longTpl-lsed: tuple-lsed.mk
	./tuple-lsed.mk Long "long int" > $@
	
long_tuple.c: $(INCL_GEN) _tuple.h longTpl-lsed
	sed -f longTpl-lsed _tuple.c > $@
	
tupleLong.h: _tuple.h longTpl-lsed
	sed -f longTpl-lsed _tuple.h > $@

tuple.h: $(INCL_TPL_GEN) tuple.h.mk
	./tuple.h.mk $(INCL_TPL_GEN) > $@

btreeDouble-lsed: btree-lsed.mk
	./btree-lsed.mk Double "double" > $@
	
double_btree.c: $(INCL_BTREE_GEN) _btree.h _btree.c btreeDouble-lsed
	sed -f btreeDouble-lsed _btree.c | sed -f double-lsed > $@
	
btreeDouble.h: _btree.h btreeDouble-lsed
	sed -f btreeDouble-lsed _btree.h | sed -f double-lsed > $@

btreeInt-lsed: btree-lsed.mk
	./btree-lsed.mk Int "int" > $@
	
int_btree.c: $(INCL_BTREE_GEN) _btree.h _btree.c btreeInt-lsed
	sed -f btreeInt-lsed _btree.c | sed -f int-lsed > $@
	
btreeInt.h: _btree.h btreeInt-lsed
	sed -f btreeInt-lsed _btree.h | sed -f int-lsed > $@

btree.h: $(INCL_BTREE_GEN)
	./btree.h.mk $(INCL_BTREE_GEN) > $@

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

clean:
	rm -f *.o $(BIN) $(LIB)

remove:		clean
	rm -f $(REMOVE)

removeAll:	remove
	rm -f $(LIBDIR)/$(LIB)
	rm -fR $(INCLUDEDIR)

