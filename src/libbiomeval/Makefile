#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
.PHONY: all mpi

# NBIS
NBISINC = -I/usr/nistlocal/nbis/include
# Hardcode path to libutil.a so it's not confused with Linux's libutil.so
NBISLIBDIR = /usr/nistlocal/nbis/lib
NBISLIB = -L$(NBISLIBDIR) -lan2k -ljpegb -lwsq -ljpegl -lfet -lioutil $(NBISLIBDIR)/libutil.a

COMMONINCOPT = -I/opt/local/include
COMMONLIBOPT = -I/opt/local/lib

# Under Linux, --libs won't print standard paths, but g++ will look in 
# -L options for appropriately named libs, so without -L standard/paths,
# NBIS image libs will be used.
OPENJPEGLIB = -L$(shell pkg-config --variable=libdir libopenjpeg) $(shell pkg-config --libs-only-l --libs-only-other libopenjpeg)
PNGLIB = -L$(shell pkg-config --variable=libdir libpng) $(shell pkg-config --libs-only-l --libs-only-other libpng)
SQLITE3LIB = -L$(shell pkg-config --variable=libdir sqlite3) $(shell pkg-config --libs-only-l --libs-only-other sqlite3)
ZLIB = -L$(shell pkg-config --variable=libdir zlib) $(shell pkg-config --libs-only-l --libs-only-other zlib)
FFMPEGLIB = -lavformat -lavutil -lswscale
COMMONLIB = $(COMMONLIBOPT) -ldb $(SQLITE3LIB) $(PNGLIB) $(OPENJPEGLIB) $(ZLIB) $(FFMPEGLIB)

include ../common.mk
include ../../VERSION

CORE = be_memory_indexedbuffer.cpp be_memory_mutableindexedbuffer.cpp be_text.cpp be_system.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_process_statistics.cpp 

IO = be_io_properties.cpp be_io_propertiesfile.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_sqliterecstore.cpp be_io_filerecstore.cpp be_io_archiverecstore.cpp be_io_compressedrecstore.cpp be_io_listrecstore.cpp be_io_recordstoreiterator.cpp be_io_utility.cpp be_io_logsheet.cpp be_io_filelogsheet.cpp be_io_syslogsheet.cpp be_io_filelogcabinet.cpp be_io_compressor.cpp be_io_gzip.cpp

IMAGE = be_image.cpp be_image_image.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_netpbm.cpp be_image_raw.cpp be_image_wsq.cpp be_image_png.cpp be_image_jpeg2000.cpp be_image_bmp.cpp

FEATURE = be_feature_minutiae.cpp be_feature_an2k7minutiae.cpp be_feature_incitsminutiae.cpp

PROCESS = be_process_worker.cpp be_process_workercontroller.cpp be_process_manager.cpp be_process_forkmanager.cpp be_process_posixthreadmanager.cpp be_process_semaphore.cpp

MESSAGE_CENTER = be_process_messagecenter.cpp be_process_mclistener.cpp be_process_mcreceiver.cpp be_process_mcutility.cpp

VIEW = be_view_view.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp

FINGER = be_finger.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_finger_an2kview_varres.cpp be_finger_an2kview_latent.cpp be_finger_an2kview_capture.cpp be_finger_incitsview.cpp be_finger_ansi2004view.cpp be_finger_ansi2007view.cpp be_finger_iso2005view.cpp

IRIS = be_iris.cpp be_iris_incitsview.cpp be_iris_iso2011view.cpp
FACE = be_face.cpp be_face_incitsview.cpp be_face_iso2005view.cpp

DATA = be_data_interchange_an2k.cpp

MPIBASE = be_mpi.cpp be_mpi_runtime.cpp be_mpi_workpackage.cpp be_mpi_workpackageprocessor.cpp be_mpi_resources.cpp be_mpi_recordstoreresources.cpp
MPIDISTRIBUTOR = be_mpi_distributor.cpp be_mpi_recordstoredistributor.cpp
MPIRECEIVER = be_mpi_receiver.cpp be_mpi_recordprocessor.cpp

VIDEO = be_video_impl.cpp be_video_container_impl.cpp be_video_stream_impl.cpp be_video_container.cpp be_video_stream.cpp

SOURCES = $(CORE) $(IO) $(PROCESS) $(IMAGE) $(FEATURE) $(VIEW) $(FINGER) $(IRIS) $(FACE) $(DATA) $(MESSAGE_CENTER) $(VIDEO)

# Source files that rely on NBIS development files being installed
NBIS_SOURCES = be_feature_an2k7minutiae.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_wsq.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_finger_an2kview_varres.cpp be_finger_an2kview_latent.cpp be_finger_an2kview_capture.cpp be_data_interchange_an2k.cpp

#
# Keep MPI related files separate so we can use a different compiler command,
# if needed.
#
MPISOURCES = $(MPIBASE) $(MPIDISTRIBUTOR) $(MPIRECEIVER)
MPIOBJECTS = $(MPISOURCES:%.cpp=%.o)

NBIS_OBJECTS = $(NBIS_SOURCES:%.cpp=%.o)
OBJECTS = $(filter-out $(NBIS_OBJECTS), $(SOURCES:%.cpp=%.o))


LIBRARY = libbiomeval
LOCALLIB = ../../lib

# MPI source files need a different compiler command
$(MPIOBJECTS): override CXX := $(MPICXX)

#
# For OS-X, we need a later version of Berkeley DB than what comes
# with the system. A check for the DB include directory is performed below.
#
all: $(OBJECTS) $(NBIS_OBJECTS) $(MPIOBJECTS)
	test -d $(LOCALLIB) || mkdir $(LOCALLIB)

ifeq ($(OS), Darwin)
	$(MPICXX) -L$(DBLIBS) $(COMMONLIB) $(filter-out $(NBIS_OBJECTS),$^) $(NBISLIB) $(NBIS_OBJECTS) -dynamiclib -Wl,-current_version,$(MAJOR_VERSION).$(MINOR_VERSION) -Wl,-compatibility_version,$(COMPATIBILITY_VERSION) -Wl,-macosx_version_min,10.6 -Wl,-install_name,$(LIBRARY).$(MAJOR_VERSION).dylib -o $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).$(MAJOR_VERSION).dylib
	$(CP) -R -P $(LIBRARY)*.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	ar rs $(LIBRARY).a $^
	ranlib $(LIBRARY).a
	$(MPICXX) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--start-group $(COMMONLIB) $(NBISLIB) -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive -Wl,--end-group -Wl,--major-image-version,$(MAJOR_VERSION) -Wl,--minor-image-version,$(MINOR_VERSION) -Wl,-soname=$(LIBRARY).dll.$(MAJOR_VERSION)
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(MPICXX) $(COMMONLIB) -lrt $(filter-out $(NBIS_OBJECTS),$^) -Wl,--start-group $(NBISLIB) $(NBIS_OBJECTS) -Wl,--end-group -shared -Wl,-soname=$(LIBRARY).so.$(MAJOR_VERSION) -o $(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION)
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
	$(CXX) $(CXXFLAGS)   -c -o $@ -DDB1X -I$(DBINCLUDE) $<
else
	$(CXX) $(CXXFLAGS)   -c -o $@ -DDB1X $<
endif

be_framework.o: CXXFLAGS += -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION)

$(NBIS_OBJECTS): CXXFLAGS := $(NBISINC) $(CXXFLAGS)
$(NBIS_OBJECTS): $(NBIS_SOURCES)

# Get include paths for libraries required third-party code
be_image_png.o: CXXFLAGS += $(shell pkg-config --cflags libpng)
be_image_jpeg2000.o: CXXFLAGS += $(shell pkg-config --cflags libopenjpeg)
be_io_gzip.o: CXXFLAGS += $(shell pkg-config --cflags zlib)
be_io_sqliterecstore.o: CXXFLAGS += $(shell pkg-config --cflags sqlite3)

ifneq ($(OS), Darwin)
be_text.o: CXXFLAGS += $(shell pkg-config --cflags libcrypto)
COMMONLIB += -L$(shell pkg-config --variable=libdir libcrypto) $(shell pkg-config --libs-only-l --libs-only-other libcrypto)
endif

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
