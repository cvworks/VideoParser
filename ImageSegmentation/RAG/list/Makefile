# Makefile for the list library

LIB_NAME = list
APP_NAME = listTest
VIDEO_PARSER_BASE = ../../..

include $(VIDEO_PARSER_BASE)/common.mk

INCS = -I..

C_SRCS = $(wildcard *.c)
HEADERS = $(wildcard *.h)

OBJS = $(C_SRCS:%.c=$(OBJFILE))

$(LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $(OBJS)

$(OBJFILE): %.c $(HEADERS)
	$(CXX) -c $(CFLAGS) $(INCS) $<

clean:
	$(RM) $(OBJS) $(LIB_FULL_NAME)
