#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
COMMONINCOPT = -I/usr/local/nbis/include
NBISLIB = -L/usr/local/nbis/lib -ljpegb -lwsq -ljpegl -lfet -lioutil -lutil -lpng -lz -lopenjpeg

include ../common.mk

CORE = be_text.cpp be_system.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_process_statistics.cpp be_utility.cpp

IO = be_io_properties.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_filerecstore.cpp be_io_archiverecstore.cpp be_io_utility.cpp be_io_logcabinet.cpp be_io_factory.cpp

IMAGE = be_image.cpp be_image_image.cpp be_image_jpeg.cpp be_image_rawimage.cpp be_image_wsq.cpp be_image_png.cpp be_image_jpeg2000.cpp

SOURCES = $(CORE) $(IO) $(IMAGE)
OBJECTS = $(SOURCES:%.cpp=%.o)
LIBRARY = libbiomeval
LOCALLIB = ../../lib

all: $(OBJECTS)
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)
ifeq ($(OS), Darwin)
	$(CXX) $(CXXFLAGS) $(NBISLIB) -dynamiclib $(OBJECTS) -lcrypto -o $(LIBRARY).dylib
	$(CP) $(LIBRARY).dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ar rs $(LIBRARY).a $(OBJECTS)
	ranlib $(LIBRARY).a
	$(CXX) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--start-group $(NBISLIB) -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive -lcrypto -ldb -Wl,--end-group
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(CXX) $(CXXFLAGS) -Wl,--start-group $(NBISLIB) $(OBJECTS) -Wl,--end-group -shared -ldb -lcrypto -o $(LIBRARY).so
	$(CP) $(LIBRARY).so $(LOCALLIB)
endif
endif

#
# The default make rule is OK for most files.
#
be_io_dbrecstore.o: be_io_dbrecstore.cpp
ifeq ($(OS), Darwin)
	$(CXX) -c $(CXXFLAGS) $<
else
	$(CXX) -c -DDB1X $(CXXFLAGS) $<
endif

be_error.o: be_error.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -D_XOPEN_SOURCE=600 $<

be_framework.o: be_framework.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) $<

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
