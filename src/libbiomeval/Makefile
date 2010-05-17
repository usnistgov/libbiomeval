#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
LOCALINC := ../include
include ../common.mk
SOURCES = be_net.c
OBJECTS = be_net.o
LIBRARY=libbiomeval

all: $(SOURCES)
ifeq ($(OS), Darwin)
	$(CC) -c $(CFLAGS) $(SOURCES)
	libtool -dynamic -o $(LIBRARY).dylib -lc $(OBJECTS) -lSystemStubs
	$(CP) $(LIBRARY).dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CC) $(CFLAGS) -c $(SOURCES)
	ar rs $(LIBRARY).a $(OBJECTS)
	ranlib $(LIBRARY).a
	$(CC) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(CC) $(CFLAGS) -shared $(SOURCES) -o $(LIB).so
	$(CP) $(LIBRARY).so $(LOCALLIB)
endif
endif

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
