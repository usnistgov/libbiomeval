#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
#
# Top-level make file to build all of the EVALUATION common code.
#
include common.mk
SRCDIRS := nbis src
INSDIRS := src

LOCALINC := $(PWD)/src/include
LOCALLIB := $(PWD)/lib

PACKAGE_DIR := libbiomeval-$(shell grep MAJOR_VERSION= VERSION | awk -F= '{print $$2}').$(shell grep MINOR_VERSION= VERSION | awk -F= '{print $$2}')

all:
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)
	@for subdir in $(SRCDIRS); do \
		(cd $$subdir && $(MAKE) all) || exit 1; \
	done

install:
	@for subdir in $(INSDIRS); do \
		(cd $$subdir && $(MAKE) install) || exit 1; \
	done

export: clean
	mkdir -p $(PACKAGE_DIR)/src
	cd $(PACKAGE_DIR) && ln -s	\
	    ../nbis 			\
	    ../common.mk 		\
	    ../Makefile 		\
	    ../VERSION 			\
	    .
	cd $(PACKAGE_DIR)/src && ln -s	\
	    ../../src/include 		\
	    ../../src/libbiomeval	\
	    ../../src/Makefile		\
	    ../../src/common.mk		\
	    .
	tar czfh $(PACKAGE_DIR).tar.gz $(PACKAGE_DIR)
	$(RM) -r $(PACKAGE_DIR)

clean:
	@for subdir in $(SRCDIRS); do \
		(cd $$subdir && $(MAKE) clean) || exit 1; \
	done
	$(RM) -r $(LOCALLIB) $(PACKAGE_DIR)
	$(RM) .gdb_history $(PACKAGE_DIR).tar.gz
