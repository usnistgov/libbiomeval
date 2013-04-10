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
SUBDIRS := src

LOCALINC := $(PWD)/src/include
LOCALLIB := $(PWD)/lib

all:
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) all) || exit 1; \
	done

install: installpaths
	install -m 644 -o $(ROOT) $(LOCALINC)/*.h $(INCPATH)
	for i in `ls $(LOCALLIB)`; do \
		cp -R $(LOCALLIB)/$$i $(LIBPATH); \
		chown root $(LIBPATH)/$$i; \
		chown 755 $(LIBPATH)/$$i; \
	done
ifeq ($(OS), Linux)
	ldconfig -n $(LIBPATH)
endif

installpaths: $(INCPATH) $(LIBPATH) $(BINPATH) $(MANPATH)
$(INCPATH):
	@echo "$(INCPATH) does not exist";
	exit 2
$(LIBPATH):
	@echo "$(LIBPATH) does not exist";
	exit 2
$(BINPATH):
	@echo "$(BINPATH) does not exist";
	exit 2
$(MANPATH):
	@echo "$(MANPATH) does not exist";
	exit 2

clean:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) clean) || exit 1; \
	done
	rm -rf $(LOCALLIB)
	rm -f .gdb_history
