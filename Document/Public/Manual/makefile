###############################################################################
# M7M01 (Eukaron) User Manual (English) -- build rules
###############################################################################

LATEXMK ?= latexmk
DOC      = rme-manual
OUTDIR   = ../..
TARGET   = $(OUTDIR)/EN_M7M01_R4T1_User_Manual.pdf
SOURCES  = $(DOC).tex rme-style.sty $(wildcard Chapter/*.tex) \
           $(wildcard Figure/*)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(LATEXMK) -xelatex -interaction=nonstopmode -halt-on-error \
		-outdir=Build $(DOC).tex
	cp Build/$(DOC).pdf $(TARGET)

clean:
	-$(LATEXMK) -C -outdir=Build $(DOC).tex
	-rm -rf Build
	-rm -f $(TARGET)
