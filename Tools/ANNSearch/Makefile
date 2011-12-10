# Make file for the ANN Library

LIB_NAME = ANN
APP_NAME = testANN
DAG_MATCHER_BASE = ..

include $(DAG_MATCHER_BASE)/common.mk

all:
	$(MAKE) ARCH="$(ARCH)" MODE="$(MODE)" -C src
	
clean:
	@$(MAKE) -s -C src clean ARCH="$(ARCH)"
	cd lib; $(RM) $(LIB_FULL_NAME)