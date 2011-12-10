CPP_SRCS = $(wildcard *.cpp)
LIBSRC = $(CPP_SRCS)
INCLUDES = $(wildcard *.h)

LIBOBJS = $(CPP_SRCS:%.cpp=%.o)
LIB = libRAG.a
BIN = testRAG
BINOBJS = $(BIN).o

ROOT = ..
LIBDIR = $(ROOT)/lib
INCDIR = $(ROOT)
INCLUDEDIR = $(ROOT)/RAG

CXX = g++
CXXFLAGS += -D__cplusplus -DLEDA_PREFIX -Wall -ffriend-injection -I$(LEDAROOT)/incl -I/usr/X11/include -I$(INCDIR) 

LDFLAGS = -L/usr/local/lib -L$(LEDAROOT) -L$(LIBDIR) -L/usr/X11R6/lib

LDLIBS = -lregion -lpnmadd -llist -lrs -ljpeg -ltiff -lnetpbm -lpng -lz \
         -lW -lP -lG -lL -lX11 -lm

CXXFLAGS += -O4

all: $(LIB) $(BIN)

#testRegion: testRegion.o
#	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

#testMerge: testMerge.o
#	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIB): $(LIBOBJS)
	ar -r $@ $(LIBOBJS)
	ranlib $@
	cp $@ $(LIBDIR)

$(BIN): $(BINOBJS) $(LIB) $(BIN).o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^
	
$(BIN).o: $(BIN).cc
	$(CXX) $(CXXFLAGS) -c $^
	
clean:
	rm -f *.o $(BIN) $(LIB)





