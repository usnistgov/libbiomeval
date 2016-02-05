#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#
COMMONINCOPT = 
include ../common.mk

CORE = test_be_time test_be_time_timer test_be_time_watchdog test_be_error test_be_error_signal_manager test_be_process_statistics test_be_system test_be_memory_autoarray test_be_text test_be_framework test_be_memory_indexedbuffer test_be_memory_orderedmap

RECORDSTORE = test_construct_be_io_filerecstore test_be_io_filerecordstore test_be_io_dbrecordstore test_be_io_sqliterecordstore test_be_io_compressedrecordstore test_be_io_filerecordstore-stress test_be_io_dbrecordstore-stress test_be_io_archiverecordstore-stress test_be_io_sqliterecordstore-stress test_construct_be_io_archiverecstore test_be_io_archiverecordstore test_be_io_listrecstore test_be_io_recordstoreunion

IO = test_be_io_filelogcabinet test_be_io_properties test_be_io_propertiesfile test_be_io_utility test_be_io_syslogsheet

IMAGE = test_be_image_raw test_be_image_jpeg test_be_image_jpegl test_be_image_jpeg2000 test_be_image_jpeg2000l test_be_image_png test_be_image_wsq test_be_image_netpbm test_be_image_bmp test_be_image_factory 

FINGER = test_be_finger_an2kview test_be_finger_an2kview_varres test_be_finger_incitsviews

IRIS = test_be_iris_incitsviews

FACE = test_be_face_incitsviews

PROCESS = test_be_process_forkmanager test_be_process_posixthreadmanager test_be_process_semaphore

COMMAND_CENTER = be_process_commandcenter_example

FEATURE = test_be_feature_an2kminutiae

OTHER = test_be_data_interchange_an2k test_be_framework_enumeration

MPI = test_be_rs_mpi test_be_csv_mpi

VIDEO = test_be_video

PROGS = $(CORE) $(IO) $(RECORDSTORE) $(IMAGE) $(VIDEO) $(PROCESS) $(FINGER) $(FEATURE) $(IRIS) $(FACE) $(OTHER) $(COMMAND_CENTER) $(MPI)

all: CXXFLAGS += -g
all: $(PROGS)

test_construct_be_io_filerecstore: test_be_io_filerecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_filerecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_dbrecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_filerecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_dbrecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_construct_be_io_archiverecstore: test_be_io_archiverecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_archiverecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_archiverecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_sqliterecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DSQLITERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_sqliterecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DSQLITERECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_compressedrecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DCOMPRESSEDRECORDSTORETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_time: test_be_time.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_time_timer: test_be_time_timer.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_time_watchdog: test_be_time_watchdog.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_filelogcabinet: test_be_io_filelogcabinet.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_properties: test_be_io_properties.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_propertiesfile: test_be_io_propertiesfile.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_utility: test_be_io_utility.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_error: test_be_error.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_error_signal_manager: test_be_error_signal_manager.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_raw: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DRAWTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpeg: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGBTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpegl: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGLTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpeg2000: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEG2000TEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpeg2000l: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEG2000LTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_netpbm: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DNETPBMTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_png: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DPNGTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_wsq: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DWSQTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_bmp: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DBMPTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_factory: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DFACTORYTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_process_statistics: test_be_process_statistics.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_system: test_be_system.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_memory_autoarray: test_be_memory_autoarray.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_text: test_be_text.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_framework: test_be_framework.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_finger_an2kview: test_be_finger_an2kview.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_iris_incitsviews: test_be_iris_incitsviews.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_face_incitsviews: test_be_face_incitsviews.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_feature_an2kminutiae: test_be_feature_an2kminutiae.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_finger_an2kview_varres: test_be_finger_an2kview_varres.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_finger_incitsviews: test_be_finger_incitsviews.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_data_interchange_an2k: test_be_data_interchange_an2k.cpp
	$(CXX) $(CXXFLAGS) $^ -pg -o $@ $(LDFLAGS) -lbiomeval
test_be_memory_indexedbuffer: test_be_memory_indexedbuffer.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_memory_orderedmap: test_be_memory_orderedmap.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_process_forkmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DFORKTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_process_posixthreadmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DPOSIXTHREADTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_process_semaphore: test_be_process_semaphore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_listrecstore: test_be_io_listrecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_framework_enumeration: test_be_framework_enumeration.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
be_process_commandcenter_example: be_process_commandcenter_example.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_rs_mpi: test_be_rs_mpi.cpp
	$(MPICXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_csv_mpi: test_be_csv_mpi.cpp
	$(MPICXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_syslogsheet: test_be_io_syslogsheet.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_video: test_be_video.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_recordstoreunion: test_be_io_recordstoreunion.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval

clean:
	$(RM) $(DISPOSABLEFILES) $(PROGS)
	$(RM) -r $(DISPOSABLEDIRS)
	$(RM) -r *_test
	$(RM) *pgm
