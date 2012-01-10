#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
ROOTNAME = BECommonFramework
SOURCETEX = intro.tex overview.tex framework.tex error.tex io.tex time.tex image.tex process.tex system.tex memory.tex text.tex feature.tex finger.tex view.tex datainterchange.tex
SOURCEBIB = $(ROOTNAME).bib
BUILDDIR = build
LATEXBUILDDIR = $(BUILDDIR)/latex
HTMLBUILDDIR = $(BUILDDIR)/html
LATEXROOTNAME = refman
LATEXPDF = $(LATEXROOTNAME).pdf
LATEXBIB = $(LATEXROOTNAME).bib
LATEXMAIN = $(ROOTNAME).tex
PDFMAIN = $(ROOTNAME).pdf
SOURCECODE = ../src/include/*.h
DOXYCONFIG = $(ROOTNAME).dox

#
# Check whether the main PDF file exists in the top-level doc directory, and
# whether it's writable. If not present, we'll just create it; if present, and
# not writable, error out. These checks accomodate having the main PDF file
# kept under version control where the local write permission is controlled
# by version control, e.g. Perforce.
#
PDFMAINPRESENT = $(shell test -f $(PDFMAIN) && echo yes)
ifeq ($(PDFMAINPRESENT),yes)
	PDFMAINWRITABLE = $(shell test -w $(PDFMAIN) && echo yes)
else
	PDFMAINWRITABLE = yes
endif

all: build $(PDFMAIN) 
build:
	mkdir -p $(LATEXBUILDDIR)
	mkdir -p $(HTMLBUILDDIR)

$(PDFMAIN): $(LATEXBUILDDIR)/$(LATEXPDF)
ifneq ($(PDFMAINWRITABLE),yes)
	$(error $(PDFMAIN) is not writable!)
else
	cp -p $(LATEXBUILDDIR)/$(LATEXPDF) $(PDFMAIN)
endif

#
# The LaTex build is a bit complicated, because Doxygen names the
# LaTex related files to start with 'refman'. In order to keep the
# bibliography file named based on our document name, it is copied
# into the build directory as refman.bib, but linked to the name we
# want.
# Also copy the non-Doxygen generated source files to the build directory.
#
$(LATEXBUILDDIR)/$(LATEXPDF): $(SOURCECODE) $(SOURCETEX) $(SOURCEBIB) $(LATEXMAIN) $(DOXYCONFIG)
	cp -f $(SOURCEBIB) $(LATEXBUILDDIR)/$(LATEXBIB)
	cp -f $(SOURCETEX) $(LATEXBUILDDIR)
	doxygen $(DOXYCONFIG)
	cd $(LATEXBUILDDIR) && pdflatex $(LATEXROOTNAME)
	cd $(LATEXBUILDDIR) && ln -sf $(LATEXBIB) $(SOURCEBIB)
	cd $(LATEXBUILDDIR) && makeindex $(LATEXROOTNAME).tex && bibtex $(LATEXROOTNAME) && pdflatex $(LATEXROOTNAME) && pdflatex $(LATEXROOTNAME)

clean:
	rm -rf build
