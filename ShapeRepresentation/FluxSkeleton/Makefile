# Make file for the Flux Skeleton Library

LIB_NAME = FluxSkel
APP_NAME = testFSL
DAG_MATCHER_BASE = ..

include $(DAG_MATCHER_BASE)/common.mk

all:
	$(MAKE) ARCH="$(ARCH)" MODE="$(MODE)" -C src
	
#headers := $(wildcard include/*.h)
#
#libsg.a: include/sg.h
#	$(MAKE) ARCH="$(ARCH)" MODE="$(MODE)" -C src
#
#include/sg.h:	$(headers)
#	cd include; rm -f sg.h;((sh make-sg_h) > sg.h)
#
#tests: libsg.a
#	$(MAKE)  -C tests

#@$(MAKE) -s -C tests clean ARCH="$(ARCH)"
	
clean:
	$(MAKE) -s -C src clean ARCH="$(ARCH)"
	cd lib; $(RM) $(LIB_FULL_NAME)