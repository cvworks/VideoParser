# Makefile for AFMM Skeleton

LIB_NAME = AFMMSkel
APP_NAME = testAFMMSkel
DAG_MATCHER_BASE = ..

include $(DAG_MATCHER_BASE)/common.mk

ifneq "$(ARCH)" "Windows"
	CXXFLAGS += -Wall -Wformat -Wno-switch -Wcast-align -Wpointer-arith -Wsynth
endif

INCS = -Iinclude -I$(DAG_MATCHER_BASE) -I$(GLUT_ROOT)
LIBS = -L$(DAG_MATCHER_BASE)/libsg/lib -L$(DAG_MATCHER_BASE)/ann_1.1/lib $(SYS_LIBS)

# Set library dependecies for the exectutable compilation (SOME MAY BE REDUNDANT OR WRONG)

LDFLAGS = -lGL -lXt -lXext -ldl -lGLU -lsg -lANN -lm -lpthread -lX11

# Collect all files

vpath %.h include

CPP_SRCS = $(wildcard *.cpp)
OBJECTS  = $(CPP_SRCS:%.cpp=$(OBJFILE))
#OBJECTS  += $(DAG_MATCHER_ROOT)/HelperFunctions.cpp=HelperFunctions.o $(DAG_MATCHER_ROOT)/ApproxPoly.cpp=ApproxPoly.o
HEADERS  = $(CPP_SRCS:.cpp=.h)

$(LIB_NAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $(OBJECTS)

test: $(OBJECTS)
	$(CXX) ../test.cxx $(DAG_MATCHER_ROOT)/ApproxPoly.cpp -$(OUT)$(APP_NAME) $(OBJECTS) $(LDFLAGS)

$(OBJFILE): %.cpp #$(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(INCS) $<

clean:
	$(RM) $(OBJECTS) $(LIB_FULL_NAME) vc80.pdb
