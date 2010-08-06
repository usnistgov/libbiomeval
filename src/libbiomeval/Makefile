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
SOURCES = be_time_timer.cpp be_error_utility.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
LIBRARY=libbiomeval

all: $(OBJECTS)

be_error_utility.o: be_error_utility.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -D_XOPEN_SOURCE=600 $<

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
