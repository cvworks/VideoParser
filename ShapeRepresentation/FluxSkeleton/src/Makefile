# Make file for the Flux Skeleton Library

LIB_NAME = FluxSkel
APP_NAME = testFSL
DAG_MATCHER_BASE = ../..

include $(DAG_MATCHER_BASE)/common.mk

INCS = -I. -I ../include
LIBS = -lm

CPP_SRCS = DiscreteSegCurve.cpp DiscreteShape.cpp LineSeg.cpp CircSeg.cpp \
	   SimpleShapeMaker.cpp SplineSmooth.cpp DivergenceMap.cpp \
       DivergenceSkeletonMaker.cpp spline_smooth.cpp SkeletonReconstruction.cpp

HEADERS := $(wildcard ../include/*.h)
OBJECTS = $(CPP_SRCS:%.cpp=$(OBJFILE))       

$(LIB_NAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $(OBJECTS)
	$(MV) $(LIB_FULL_NAME) ../lib

$(OBJFILE): %.cpp #$(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(INCS) $<

clean:
	@echo "Cleaning src directory..."
	@$(RM) $(OBJECTS) $(LIB_FULL_NAME) vc80.pdb


