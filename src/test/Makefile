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
PROGS = test_construct_be_io_filerecstore test_be_io_filerecordstore test_be_io_dbrecordstore test_be_io_filerecordstore-stress test_be_io_dbrecordstore-stress test_be_io_archiverecordstore-stress test_construct_be_io_archiverecstore test_be_io_archiverecordstore test_be_time_timer test_be_time_watchdog test_be_io_logcabinet test_be_io_properties test_be_io_factory test_be_error_signal_manager test_be_image_rawimage test_be_image_jpeg test_be_process_statistics test_be_system test_be_utility_autoarray test_be_text test_be_utility test_be_framework

all: CXXFLAGS += -g
all: $(PROGS)

test_be_netserver: test_be_netserver.c
	$(CC) $(CFLAGS) $^ -o $@ -lbiomeval
test_be_netclient: test_be_netclient.c
	$(CC) $(CFLAGS) $^ -o $@ -lbiomeval
test_construct_be_io_filerecstore: test_be_io_filerecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_io_filerecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_io_dbrecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ -lbiomeval
test_be_io_filerecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_io_dbrecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ -lbiomeval
test_construct_be_io_archiverecstore: test_be_io_archiverecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_io_archiverecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_io_archiverecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_time_timer: test_be_time_timer.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_time_watchdog: test_be_time_watchdog.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_io_logcabinet: test_be_io_logcabinet.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_io_properties: test_be_io_properties.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_error_signal_manager: test_be_error_signal_manager.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_image_rawimage: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DRAWIMAGETEST $^ -o $@ -lbiomeval
test_be_image_jpeg: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGTEST $^ -o $@ -lbiomeval
test_be_io_factory: test_be_io_factory.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_process_statistics: test_be_process_statistics.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_system: test_be_system.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_utility_autoarray: test_be_utility_autoarray.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_text: test_be_text.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_utility: test_be_utility.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_framework: test_be_framework.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval

clean:
	$(RM) $(DISPOSABLEFILES) $(PROGS)
	$(RM) -r $(DISPOSABLEDIRS)
	$(RM) -rf *_test
