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
SOURCES = be_error_exception.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_filerecstore.cpp be_io_archiverecstore.cpp be_io_utility.cpp be_io_logcabinet.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)

#
# The default make rule is OK for most files.
#
all: $(OBJECTS)

be_io_dbrecstore.o: be_io_dbrecstore.cpp
ifeq ($(OS), Darwin)
	$(CXX) -c $(CXXFLAGS) $<
else
	$(CXX) -c -DDB1X $(CXXFLAGS) $<
endif

clean:
	$(RM) $(DISPOSABLEFILES)
	$(RM) -r $(DISPOSABLEDIRS)
