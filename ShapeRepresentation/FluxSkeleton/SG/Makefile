BIN = main
CC = g++ $(FLAGS) -ggdb -DLEDA_PREFIX -D_DEBUG -Wno-deprecated
HNSRTREE_ROOT = $(DAG_MATCHER_ROOT)/HnSRTree-2.0beta4

INCS = -I../include \
       -I$(DAG_MATCHER_ROOT)/dagmatcher \
       -I$(DAG_MATCHER_ROOT)/dagmatcher/common \
       -I$(NEWMAT_ROOT) \
       -I$(LEDAROOT)/incl/ \
       -I$(SHOCKGRAPH_ROOT)/read-xml \
       -I$(SHOCKGRAPH_ROOT)/backend \
       -I$(SHOCKGRAPH_ROOT)/helpercode \
       -I$(SHOCKGRAPH_ROOT)/common \
       -I$(SHOCKGRAPH_ROOT)/interfaces \
       -I$(SHOCKGRAPH_ROOT)/geno \
       -I$(HNSRTREE_ROOT)/include \
       -I/usr/include/gnome-xml

LIBS = -L../lib \
       -L$(DAG_MATCHER_ROOT)/dagmatcher \
       -L$(DAG_MATCHER_BASE)/lib \
       -L$(NEWMAT_ROOT) \
       -L$(LEDAROOT) \
       -L$(SHOCKGRAPH_ROOT)/lib \
       -L$(SHOCKGRAPH_ROOT)/interfaces \
       -L$(HNSRTREE_ROOT)/lib \
       -L/../usr/X11R6/lib/

# INCS =  -I. \
# 	-I/usr/include \
# 	-I../include \
# 	-I$(LEDAROOT)/incl

# LIBS = -L../lib \
# 	-L/usr/lib \
# 	-L/usr/X11R6/lib

LEDA_LIBS = -lW -lP -lG -lL -lX11
SHOCKLIB = -lsg
PPM_LIBS = -lppm -lpgm -lpbm
DAG_LIBS = -lDAGMatch -lHnSRTree

test: main.o
	$(CC) -o $(BIN) $(LIBS) main.o $(DAG_LIBS) $(LEDA_LIBS) -lnewmat $(SHOCKLIB) -lm $(PPM_LIBS)

main.o: main.cpp
	$(CC) -c $(INCS) main.cpp
	
clean:
	rm *.o $(BIN)
clobber:
	rm *.o $(BIN) *~
