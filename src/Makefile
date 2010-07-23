#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#

# The 'core' library and programs, that always build
CORE := libbiomeval io
SUBDIRS := $(CORE)

OBJECTS := io/be_exception.o io/be_recordstore.o io/be_dbrecstore.o io/be_filerecstore.o io/be_archiverecstore.o libbiomeval/be_net.o libbiomeval/be_timer.o
LIBRARY := libbiomeval
LOCALLIB := ../lib

include ../common.mk

all:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) all) || exit 1; \
	done
ifeq ($(OS), Darwin)
	$(CXX) $(CXXFLAGS) -dynamiclib $(OBJECTS) -o $(LIBRARY).dylib
	$(CP) $(LIBRARY).dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ar rs $(LIBRARY).a $(OBJECTS)
	ranlib $(LIBRARY).a
	$(CXX) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(CXX) $(CXXFLAGS) -shared $(OBJECTS) -ldb -o $(LIBRARY).so
	$(CP) $(LIBRARY).so $(LOCALLIB)
endif
endif

clean:
	@for subdir in $(SUBDIRS); do \
		(cd $$subdir && $(MAKE) clean) || exit 1; \
	done
	rm -f .gdb_history
