#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
.PHONY: all

COMMONINCOPT = -I/usr/local/nbis/include
NBISLIB = -L/usr/local/nbis/lib -lan2k -ljpegb -lwsq -ljpegl -lfet -lioutil -lutil -lpng -lz -lopenjpeg

include ../common.mk
include ../../VERSION

CORE = be_memory_indexedbuffer.cpp be_text.cpp be_system.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_process_statistics.cpp 

IO = be_io_properties.cpp be_io_propertiesfile.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_sqliterecstore.cpp be_io_filerecstore.cpp be_io_archiverecstore.cpp be_io_compressedrecstore.cpp be_io_utility.cpp be_io_logsheet.cpp be_io_logcabinet.cpp be_io_compressor.cpp be_io_gzip.cpp

IMAGE = be_image.cpp be_image_image.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_netpbm.cpp be_image_raw.cpp be_image_wsq.cpp be_image_png.cpp be_image_jpeg2000.cpp

FEATURE = be_feature_minutiae.cpp be_feature_an2k7minutiae.cpp be_feature_incitsminutiae.cpp

PROCESS_MANAGEMENT = be_process_worker.cpp be_process_workercontroller.cpp be_process_manager.cpp be_process_forkmanager.cpp be_process_posixthreadmanager.cpp

VIEW = be_view_view.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp

FINGER = be_finger.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_finger_an2kview_varres.cpp be_finger_an2kview_latent.cpp be_finger_an2kview_capture.cpp be_finger_incitsview.cpp be_finger_ansi2004view.cpp be_finger_ansi2007view.cpp be_finger_iso2005view.cpp

DATA = be_data_interchange_an2k.cpp

SOURCES = $(CORE) $(IO) $(PROCESS_MANAGEMENT) $(IMAGE) $(FEATURE) $(VIEW) $(FINGER) $(DATA)
OBJECTS = $(SOURCES:%.cpp=%.o)
LIBRARY = libbiomeval
LOCALLIB = ../../lib

#
# For OS-X, we need a later version of Berkeley DB than what comes
# with the system. A check for the DB include directory is performed below.
#
all: $(OBJECTS)
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)

ifeq ($(OS), Darwin)
	$(CXX) $(CXXFLAGS) $(NBISLIB) -L$(DBLIBS) -ldb -lsqlite3 -lcrypto -dynamiclib $(OBJECTS) -Wl,-current_version,$(MAJOR_VERSION).$(MINOR_VERSION) -Wl,-compatibility_version,$(COMPATIBILITY_VERSION) -Wl,-macosx_version_min,10.6 -Wl,-install_name,$(LIBRARY).$(MAJOR_VERSION).dylib -o $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).$(MAJOR_VERSION).dylib
	$(CP) -R -P $(LIBRARY)*.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ar rs $(LIBRARY).a $(OBJECTS)
	ranlib $(LIBRARY).a
	$(CXX) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--start-group $(NBISLIB) -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive -lcrypto -ldb -lsqlite3 -Wl,--end-group -Wl,--major-image-version,$(MAJOR_VERSION) -Wl,--minor-image-version,$(MINOR_VERSION) -Wl,-soname=$(LIBRARY).dll.$(MAJOR_VERSION)
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(CXX) $(CXXFLAGS) -Wl,--start-group $(NBISLIB) $(OBJECTS) -Wl,--end-group -shared -ldb -lcrypto -lsqlite3 -lrt -Wl,-soname=$(LIBRARY).so.$(MAJOR_VERSION) -o $(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION)
	/sbin/ldconfig -n $(PWD)
	ln -f -s $(LIBRARY).so.$(MAJOR_VERSION) $(LIBRARY).so
	$(CP) -P $(LIBRARY).so* $(LOCALLIB)
endif
endif

#
# The default make rule is OK for most files.
#
DBINCLUDE = /opt/local/include/db44
DBLIBS = /opt/local/lib/db44
be_io_dbrecstore.o: be_io_dbrecstore.cpp
ifeq ($(OS), Darwin)
	$(if $(wildcard $(DBINCLUDE)),\
	,\
	$(error Berkeley DB include directory not found!))
	$(CXX) -c -DDB1X -I $(DBINCLUDE) $(CXXFLAGS) $<
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
