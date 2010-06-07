#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
include ../common.mk
LIBRARY = libbiomevalio
SOURCES = be_exception.cpp be_recordstore.cpp be_dbrecstore.cpp be_filerecstore.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)

all: $(OBJECTS)
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

be_dbrecstore.o: be_dbrecstore.cpp
ifeq ($(OS), Darwin)
	$(CXX) -c $(CXXFLAGS) $<
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(CXX) -c -DDB1X $(CXXFLAGS) $<
else
	$(CXX) -c -DDB1X $(CXXFLAGS) $<
endif
endif

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
