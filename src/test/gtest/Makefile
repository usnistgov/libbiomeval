#
# This software was developed at the National Institute of Standards and
# Technology (NIST) by employees of the Federal Government in the course
# of their official duties. Pursuant to title 17 Section 105 of the
# United States Code, this software is not subject to copyright protection
# and is in the public domain. NIST assumes no responsibility  whatsoever for
# its use by other parties, and makes no guarantees, expressed or implied,
# about its quality, reliability, or any other characteristic.
#

COMMONINCOPT = -I../../../../../../../vendor/google/gtest/include
include common.mk
LDFLAGS += -lbiomeval -L../../../../../../../vendor/google/gtest -lgtest_main -lgtest

CORE = test_be_time_timer test_be_time test_be_time_watchdog test_be_text test_be_error test_be_error_signal_manager test_be_memory_autoarray test_be_memory_indexedbuffer test_be_memory_mutableindexedbuffer test_be_memory_orderedmap test_be_framework_enumeration test_be_framework

FACE = test_be_face_incitsviews

FINGER = test_be_finger_an2kview_fixedres test_be_finger_an2kview_varres test_be_finger_incitsviews

IMAGE = test_be_image_jpeg test_be_image_jpegl test_be_image_jpeg2000 test_be_image_jpeg2000l test_be_image_png test_be_image_netpbm test_be_image_bmp test_be_image_wsq test_be_image_factory test_be_image_raw

IO = test_be_io_filerecordstore test_be_io_dbrecordstore test_be_io_sqliterecordstore test_be_io_compressedrecordstore test_be_io_archiverecordstore test_be_io_utility test_be_io_properties test_be_io_propertiesfile test_be_io_archiverecordstore-stress test_be_io_dbrecordstore-stress test_be_io_sqliterecordstore-stress test_be_io_filerecordstore-stress

IRIS = test_be_iris_incitsviews

PROCESS = test_be_process_semaphore test_be_process_forkmanager test_be_process_posixthreadmanager

PROGS = $(CORE) $(FACE) $(FINGER) $(IMAGE) $(IO) $(IRIS) $(PROCESS)

all: CXXFLAGS += -g
all: $(PROGS)

# Need special rules for tests that require defines.
test_be_io_filerecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_dbrecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_archiverecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_sqliterecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DSQLITERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_compressedrecordstore: test_be_io_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DCOMPRESSEDRECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_filerecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_dbrecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_archiverecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_io_sqliterecordstore-stress: test_be_io_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DSQLITERECORDSTORETEST $^ -o $@ $(LDFLAGS)
test_be_image_wsq: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DWSQTEST $^ -o $@ $(LDFLAGS)
test_be_image_jpeg: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGBTEST $^ -o $@ $(LDFLAGS)
test_be_image_jpegl: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEGLTEST $^ -o $@ $(LDFLAGS)
test_be_image_png: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DPNGTEST $^ -o $@ $(LDFLAGS)
test_be_image_jpeg2000: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEG2000TEST $^ -o $@ $(LDFLAGS)
test_be_image_jpeg2000l: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DJPEG2000LTEST $^ -o $@ $(LDFLAGS)
test_be_image_bmp: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DBMPTEST $^ -o $@ $(LDFLAGS)
test_be_image_netpbm: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DNETPBMTEST $^ -o $@ $(LDFLAGS)
test_be_image_raw: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DRAWTEST $^ -o $@ $(LDFLAGS)
test_be_image_factory: test_be_image_image.cpp
	$(CXX) $(CXXFLAGS) -DFACTORYTEST $^ -o $@ $(LDFLAGS)
test_be_process_forkmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DFORK $^ -o $@ $(LDFLAGS)
test_be_process_posixthreadmanager: test_be_process_manager.cpp
	$(CXX) $(CXXFLAGS) -DTHREAD $^ -o $@ $(LDFLAGS)
test_be_io_propertiesfile: test_be_io_properties.cpp
	$(CXX) $(CXXFLAGS) -DPROPERTIESFILE $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(DISPOSABLEFILES) $(PROGS)
	$(RM) -r $(DISPOSABLEDIRS)

# Run all tests, except stress tests
test:
	$(shell for unittest in `find . -type f -perm +111 -print |	\
		grep -v stress`;					\
		do							\
			./$$unittest > /dev/null;			\
			if [ $$? -ne 0 ]; then				\
				echo `basename $$unittest` FAIL;	\
			fi						\
		done >&2)

# Run absolutely every test
testall:
	$(shell for unittest in `find . -type f -perm +111 -print |	\
		do							\
			./$$unittest > /dev/null;			\
			if [ $$? -ne 0 ]; then				\
				echo `basename $$unittest` FAIL;	\
			fi						\
		done >&2)


