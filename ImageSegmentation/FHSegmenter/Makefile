# Makefile for the Graph-Based Segmentation code

LIB_NAME = GBSegment
APP_NAME = GBSegmentTest
VIDEO_PARSER_BASE = ../..

include $(VIDEO_PARSER_BASE)/common.mk

INCS = -I$(VXL_ROOT)/core \
       -I$(VXL_ROOT)/vcl \
       -I$(VXL_ROOT)/vcl/config.win32/vc70 \
       -I$(VXL_ROOT)/core/vnl \
       -I$(FLTK_ROOT)/include \
       -I$(VIDEO_PARSER_BASE)

C_SRCS = $(wildcard *.c)
CPP_SRCS = $(wildcard *.cpp)
FL_SRCS = $(wildcard *.fl)
HEADERS = $(wildcard *.h)

OBJS = $(C_SRCS:%.c=$(OBJFILE)) $(CC_SRCS:%.cc=$(OBJFILE)) $(CXX_SRCS:%.cxx=$(OBJFILE)) \
       $(CPP_SRCS:%.cpp=$(OBJFILE)) $(FL_SRCS:%.fl=$(OBJFILE))

$(LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $(OBJS)

$(OBJFILE): %.c $(HEADERS)
	$(CXX) -c $(CFLAGS) $(INCS) $<

$(OBJFILE): %.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(INCS) $<

$(OBJFILE): %.cxx $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $(INCS) $<

%.cxx: %.fl
	$(FLUID) -c $<

clean:
	$(RM) $(OBJS) $(LIB_FULL_NAME)
