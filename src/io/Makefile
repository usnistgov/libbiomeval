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
SOURCES = be_exception.cpp be_recordstore.cpp be_dbrecstore.cpp be_filerecstore.cpp be_archiverecstore.cpp be_io_utility.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)

all: $(OBJECTS)

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
