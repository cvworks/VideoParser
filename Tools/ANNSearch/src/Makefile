# Make file for the ANN Library

LIB_NAME = ANN
APP_NAME = testANN
DAG_MATCHER_BASE = ../..

include $(DAG_MATCHER_BASE)/common.mk

INCS = -I. -I ../include
LIBS = -lm

# USe: -DDLL_EXPORTS (for compiling dll), -D_LIB (for compiling or linking against a library)
#      or nothing (for linking against a dll )
CXXFLAGS += -DANN_PERF -DANN_NO_RANDOM -D_LIB

CPP_SRCS = kd_fix_rad_search.cpp kd_dump.cpp bd_fix_rad_search.cpp \
           perf.cpp kd_util.cpp kd_tree.cpp kd_split.cpp kd_search.cpp kd_pr_search.cpp \
           brute.cpp bd_tree.cpp bd_search.cpp bd_pr_search.cpp ANN.cpp

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


