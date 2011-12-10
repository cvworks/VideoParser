# Makefile for the LIBSG documentation
#--------------------------------------------------------------------
#
# COMMON DEFINITIONS
#
include ../Common
#
# FILES TO BE TESTED
#
headers := $(wildcard ../web/*.w)
#headers += $(wildcard ../src/functional/*.w)
#headers += $(wildcard ../src/linesearch/*.w)
#headers += $(wildcard ../src/barrier/*.w)
#headers += $(wildcard ../src/searchdir/*.w)
#
# TARGETS
#
all: libsg

.PHONY: all clean libsg manual pdf

libsg: libsg.dvi

manual: manual.dvi

ps: libsg.dvi
	@echo "Making PS file of source documentation..."
	@$(DVIPS) libsg.dvi -o 2> /dev/null

pdf: libsg.pdf
#	@echo "Making PDF file of source documentation..."
#	@$(DVIPDFM) libsg.dvi

libsg.dvi: libsg.web $(headers)
	@echo "Making dvi file of source documentation..."
	$(CWEAVE) -p libsg.web
	$(TEX) libsg
	@$(RM) libsg.tex

libsg.pdf: libsg.web $(headers)
	@echo "Making pdf file of source documentation..."
	$(CWEAVE) -p libsg.web
	pdf$(TEX) libsg
	@$(RM) libsg.tex


manual.dvi: manual.tex 
	@echo "Making manual..."
	tex manual

clean:
	@echo "Cleaning doc directory..."
	@$(RM) *~ *.log *.idx *.scn *.toc libsg.tex *.dvi *.ps *.pdf *.aux

%:
	$(MAKE) -s -C .. $@
