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

# Must use the absolute path to the NBIS static libraries
# System install of NBIS
#NBISINC = -I/usr/local/nbis/include
#NBISLIBDIR = /usr/local/nbis/lib
#NBISLIB = -L$(NBISLIBDIR) -lan2k -ljpegb -lwsq -ljpegl -lfet -lioutil $(NBISLIBDIR)/libutil.a

# Use locally built NBIS subset
NBISINC = -I../../nbis/include/
NBISLIBDIR = $(PWD)/../../lib
NBISLIB = -L$(NBISLIBDIR) -l$(NBISEVALNAME)

# Paths to static NBIS libraries, either system or local
NBISSTATIC = $(patsubst -l%,$(NBISLIBDIR)/lib%.a,$(filter-out -L%,$(NBISLIB)))

COMMONINCOPT = -I/opt/local/include
#
# On Linux, winscard.h may #include<pcsclite.h>, so we need the complete
# path to that file.
#
COMMONINCOPT += -I/usr/include/PCSC
COMMONLIBOPT = -I/opt/local/lib

# Includes must be after setting COMMON* above
include ../common.mk
include ../../VERSION

# Under Linux, --libs won't print standard paths, but g++ will look in
# -L options for appropriately named libs, so without -L standard/paths,
# NBIS image libs will be used.
# Use the installed JPEG lib, not that from NBIS as we rely on the NBIS subset
# built within the Framework.
JPEGLIB = -ljpeg
OPENJPEGLIB = -L$(shell PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --variable=libdir libopenjp2) $(shell PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --libs-only-l --libs-only-other libopenjp2)
PNGLIB = -L$(shell pkg-config --variable=libdir libpng) $(shell pkg-config --libs-only-l --libs-only-other libpng)
SQLITE3LIB = -L$(shell pkg-config --variable=libdir sqlite3) $(shell pkg-config --libs-only-l --libs-only-other sqlite3)
TIFFLIB = $(shell pkg-config --libs libtiff-4)
ZLIB = -L$(shell pkg-config --variable=libdir zlib) $(shell pkg-config --libs-only-l --libs-only-other zlib)
FFMPEGLIB = -lavformat -lavutil -lswscale -lavcodec

COMMONLIB = $(COMMONLIBOPT) -ldb $(SQLITE3LIB) $(PNGLIB) $(JPEGLIB) $(OPENJPEGLIB) $(TIFFLIB) $(ZLIB) $(FFMPEGLIB) $(PCSCLIB)

ifneq ($(OS), Darwin)
PCSCLIB = -lpcsclite
else
PCSCLIB = -framework PCSC
endif

CORE = be_memory_indexedbuffer.cpp be_memory_mutableindexedbuffer.cpp be_text.cpp be_system.cpp be_error.cpp be_error_exception.cpp be_time.cpp be_time_timer.cpp be_time_watchdog.cpp be_error_signal_manager.cpp be_framework.cpp be_framework_status.cpp be_framework_api.cpp be_process_statistics.cpp

IO = be_io_properties.cpp be_io_propertiesfile.cpp be_io_utility.cpp be_io_logsheet.cpp be_io_filelogsheet.cpp be_io_syslogsheet.cpp be_io_filelogcabinet.cpp be_io_compressor.cpp be_io_gzip.cpp

RECORDSTORE = be_io_recordstore_impl.cpp be_io_recordstore.cpp be_io_dbrecstore.cpp be_io_dbrecstore_impl.cpp be_io_sqliterecstore.cpp be_io_sqliterecstore_impl.cpp be_io_filerecstore.cpp be_io_filerecstore_impl.cpp be_io_listrecstore.cpp be_io_listrecstore_impl.cpp be_io_archiverecstore.cpp be_io_archiverecstore_impl.cpp be_io_compressedrecstore_impl.cpp be_io_compressedrecstore.cpp be_io_recordstoreunion.cpp be_io_recordstoreunion_impl.cpp be_io_persistentrecordstoreunion.cpp be_io_persistentrecordstoreunion_impl.cpp

IMAGE = be_image.cpp be_image_image.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_netpbm.cpp be_image_raw.cpp be_image_wsq.cpp be_image_png.cpp be_image_jpeg2000.cpp be_image_bmp.cpp be_image_tiff.cpp

FEATURE = be_feature.cpp be_feature_minutiae.cpp be_feature_an2k7minutiae.cpp be_feature_incitsminutiae.cpp be_feature_sort.cpp be_feature_an2k11efs.cpp be_feature_an2k11efs_impl.cpp

PROCESS = be_process_worker.cpp be_process_workercontroller.cpp be_process_manager.cpp be_process_forkmanager.cpp be_process_posixthreadmanager.cpp be_process_semaphore.cpp

MESSAGE_CENTER = be_process_messagecenter.cpp be_process_mclistener.cpp be_process_mcreceiver.cpp be_process_mcutility.cpp

VIEW = be_view_view.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp

FINGER = be_finger.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_latent_an2kview.cpp be_finger_an2kview_capture.cpp be_finger_incitsview.cpp be_finger_ansi2004view.cpp be_finger_ansi2007view.cpp be_finger_iso2005view.cpp be_data_interchange_finger.cpp

PALM = be_palm.cpp be_palm_an2kview.cpp
PLANTAR = be_plantar.cpp

IRIS = be_iris.cpp be_iris_incitsview.cpp be_iris_iso2011view.cpp
FACE = be_face.cpp be_face_incitsview.cpp be_face_iso2005view.cpp

DATA = be_data_interchange_an2k.cpp be_data_interchange_ansi2004.cpp

MPIBASE = be_mpi.cpp be_mpi_csvresources.cpp be_mpi_exception.cpp be_mpi_runtime.cpp be_mpi_workpackage.cpp be_mpi_workpackageprocessor.cpp be_mpi_resources.cpp be_mpi_recordstoreresources.cpp
MPIDISTRIBUTOR = be_mpi_distributor.cpp be_mpi_recordstoredistributor.cpp be_mpi_csvdistributor.cpp
MPIRECEIVER = be_mpi_receiver.cpp be_mpi_recordprocessor.cpp be_mpi_csvprocessor.cpp

VIDEO = be_video_impl.cpp be_video_container_impl.cpp be_video_stream_impl.cpp be_video_container.cpp be_video_stream.cpp

DEVICE = be_device_tlv_impl.cpp be_device_tlv.cpp be_device_smartcard_impl.cpp be_device_smartcard.cpp

SOURCES = $(CORE) $(IO) $(RECORDSTORE) $(PROCESS) $(IMAGE) $(FEATURE) $(VIEW) $(FINGER) $(PALM) $(PLANTAR) $(IRIS) $(FACE) $(DATA) $(MESSAGE_CENTER) $(VIDEO) $(DEVICE)

# Source files that rely on NBIS development files being installed
NBIS_SOURCES = be_feature_an2k7minutiae.cpp be_feature_an2k11efs_impl.cpp be_image_jpeg.cpp be_image_jpegl.cpp be_image_wsq.cpp be_view_an2kview.cpp be_view_an2kview_varres.cpp be_finger_an2kminutiae_data_record.cpp be_finger_an2kview.cpp be_finger_an2kview_fixedres.cpp be_latent_an2kview.cpp be_finger_an2kview_capture.cpp be_palm_an2kview.cpp be_data_interchange_an2k.cpp

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

# All platforms get a static library
# Pull the NBIS .o files from the NBIS libs into the Evaluation lib
	ar cq $(LIBRARY).a $^
	dir=`mktemp -d $(LIBRARY).XXXXXX`; for lib in $(NBISSTATIC); do cd $$dir; ar x $$lib; ar q ../$(LIBRARY).a *; rm *; cd ..; done; rmdir $$dir
	ranlib $(LIBRARY).a
	$(CP) -R -P $(LIBRARY).a $(LOCALLIB)
# Create shared libs
ifeq ($(OS), Darwin)
	$(MPICXX) $(filter-out $(NBIS_OBJECTS),$^) $(NBISLIB) $(NBIS_OBJECTS) -L$(DBLIBS) $(COMMONLIB) -dynamiclib -Wl,-current_version,$(MAJOR_VERSION).$(MINOR_VERSION) -Wl,-compatibility_version,$(COMPATIBILITY_VERSION) -Wl,-macosx_version_min,$(OSX_MAJOR).$(OSX_MINOR) -Wl,-install_name,$(LIBRARY).$(MAJOR_VERSION).dylib -o $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).dylib
	ln -f -s $(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBRARY).$(MAJOR_VERSION).dylib
	$(CP) -R -P $(LIBRARY)*.dylib $(LOCALLIB)
else
ifeq ($(findstring CYGWIN,$(OS)), CYGWIN)
	$(MPICXX) -shared -o $(LIBRARY).dll -Wl,--out-implib=$(LIBRARY).dll.a -Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--start-group $(COMMONLIB) $(NBISLIB) -Wl,--whole-archive $(LIBRARY).a -Wl,--no-whole-archive -Wl,--end-group -Wl,--major-image-version,$(MAJOR_VERSION) -Wl,--minor-image-version,$(MINOR_VERSION) -Wl,-soname=$(LIBRARY).dll.$(MAJOR_VERSION)
	$(CP) $(LIBRARY).a $(LOCALLIB)
	$(CP) $(LIBRARY).dll.a $(LOCALLIB)
	$(CP) $(LIBRARY).dll $(LOCALLIB)
else
	$(MPICXX) $(filter-out $(NBIS_OBJECTS),$^) -Wl,--start-group $(NBISLIB) $(NBIS_OBJECTS) -Wl,--end-group -shared $(COMMONLIB) -lrt -Wl,-soname=$(LIBRARY).so.$(MAJOR_VERSION) -o $(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION)
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
be_io_dbrecstore_impl.o: be_io_dbrecstore_impl.cpp
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
be_image_tiff.o: CXXFLAGS += $(shell pkg-config --cflags libtiff-4)
be_image_png.o: CXXFLAGS += $(shell pkg-config --cflags libpng)
be_image_jpeg2000.o: CXXFLAGS += $(shell PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig pkg-config --cflags libopenjp2)
be_io_gzip.o: CXXFLAGS += $(shell pkg-config --cflags zlib)
be_io_sqliterecstore.o: CXXFLAGS += $(shell pkg-config --cflags sqlite3)

ifeq ($(OS), Darwin)
COMMONLIB += -framework Foundation -framework Security
else
be_text.o: CXXFLAGS += $(shell pkg-config --cflags libcrypto)
COMMONLIB += -L$(shell pkg-config --variable=libdir libcrypto) $(shell pkg-config --libs-only-l --libs-only-other libcrypto)
endif

install: installpaths
	install -m 644 -o $(ROOT) $(LOCALINC)/*.h $(INCPATH)
	$(CP) -pf $(LIBRARY).a $(LIBPATH)
ifeq ($(OS), Linux)
	$(CP) -pf $(LOCALLIB)/$(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION) $(LIBPATH)
	ln -sf $(LIBPATH)/$(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION) $(LIBPATH)/$(LIBRARY).so.$(MAJOR_VERSION)
	ln -sf $(LIBPATH)/$(LIBRARY).so.$(MAJOR_VERSION).$(MINOR_VERSION) $(LIBPATH)/$(LIBRARY).so
	ldconfig -n $(LIBPATH)
else
ifeq ($(OS), Darwin)
	$(CP) -pf $(LOCALLIB)/$(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBPATH)
	ln -sf $(LIBPATH)/$(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBPATH)/$(LIBRARY).$(MAJOR_VERSION).dylib
	ln -sf $(LIBPATH)/$(LIBRARY).$(MAJOR_VERSION).$(MINOR_VERSION).dylib $(LIBPATH)/$(LIBRARY).dylib
else
	install -m 755 -o $(ROOT) $(LOCALLIB)/$(LIBRARY).* $(LIBPATH)
endif
endif
	chown $(ROOT) $(LIBPATH)/$(LIBRARY).*
	chmod 755 $(LIBPATH)/$(LIBRARY).*

installpaths: $(INCPATH) $(LIBPATH) $(BINPATH) $(MANPATH)
$(INCPATH):
	$(error $(INCPATH) does not exist)
$(LIBPATH):
	$(error $(LIBPATH) does not exist)
$(BINPATH):
	$(error $(BINPATH) does not exist)
$(MANPATH):
	$(error $(MANPATH) does not exist)

clean:
	$(RM) $(DISPOSABLEFILES) $(LIBRARY).*
	$(RM) -r $(DISPOSABLEDIRS)
