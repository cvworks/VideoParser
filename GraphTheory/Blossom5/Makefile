DIRS := . MinCost GEOM 

SOURCES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS := $(patsubst %.cpp, %.o, $(SOURCES))

CFLAGS := -O5 -D_NDEBUG
CC := c++
LIBS :=  -lrt 
INCLUDES := 
LIBDIR := 

all: blossom5

blossom5: ${OBJS}
	${CC} ${CFLAGS} ${LIBDIR} -o $@ ${OBJS} ${LIBS}

.cpp.o:
	$(CC) $(CFLAGS) ${INCLUDES} $< -c -o $@

clean:
	rm -f ${OBJS} blossom5
