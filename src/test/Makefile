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
include ../common.mk

CORE = test_be_time test_be_time_timer test_be_time_watchdog test_be_error_signal_manager test_be_process_statistics test_be_system test_be_memory_autoarray test_be_text test_be_framework test_be_memory_indexedbuffer

IO = test_construct_be_io_filerecstore test_be_io_filerecordstore test_be_io_dbrecordstore test_be_io_filerecordstore-stress test_be_io_dbrecordstore-stress test_be_io_archiverecordstore-stress test_construct_be_io_archiverecstore test_be_io_archiverecordstore test_be_io_logcabinet test_be_io_properties test_be_io_utility

IMAGE = test_be_image_rawimage test_be_image_jpeg test_be_image_jpegl test_be_image_jpeg2000 test_be_image_png test_be_image_wsq test_be_image_netpbm

FINGER = test_be_finger_an2kview test_be_finger_an2kview_varres test_be_finger_incitsviews

PROCESS_MANAGEMENT = test_be_process_forkmanager test_be_process_posixthreadmanager

FEATURE = test_be_feature_an2kminutiae

OTHER = test_be_data_interchange_an2k

PROGS = $(CORE) $(IO) $(IMAGE) $(PROCESS_MANAGEMENT) $(FINGER) $(FEATURE) $(OTHER)

all: CXXFLAGS += -g
all: $(PROGS)

test_be_netserver: test_be_netserver.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_netclient: test_be_netclient.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
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
test_be_time: test_be_time.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_time_timer: test_be_time_timer.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_time_watchdog: test_be_time_watchdog.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_logcabinet: test_be_io_logcabinet.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_properties: test_be_io_properties.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_io_utility: test_be_io_utility.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_error_signal_manager: test_be_error_signal_manager.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_rawimage: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DRAWIMAGETEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpeg: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGBTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpegl: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGLTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_jpeg2000: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEG2000TEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_netpbm: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DNETPBMTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_png: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DPNGTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_image_wsq: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DWSQTEST $^ -o $@ $(LDFLAGS) -lbiomeval
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
test_be_process_forkmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DFORKTEST $^ -o $@ $(LDFLAGS) -lbiomeval
test_be_process_posixthreadmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DPOSIXTHREADTEST $^ -o $@ $(LDFLAGS) -lbiomeval

clean:
	$(RM) $(DISPOSABLEFILES) $(PROGS)
	$(RM) -r $(DISPOSABLEDIRS)
	$(RM) -r *_test
	$(RM) *pgm
