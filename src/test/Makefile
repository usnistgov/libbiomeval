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
#PROGS = test_be_netserver test_be_netclient test_construct_be_filerecstore test_be_filerecordstore test_be_dbrecordstore test_be_filerecordstore-stress test_be_dbrecordstore-stress test_be_archiverecordstore-stress test_construct_be_archiverecstore test_be_archiverecordstore test_be_timer
PROGS = test_construct_be_filerecstore test_be_filerecordstore test_be_dbrecordstore test_be_filerecordstore-stress test_be_dbrecordstore-stress test_be_archiverecordstore-stress test_construct_be_archiverecstore test_be_archiverecordstore test_be_timer

all: $(PROGS)

test_be_netserver: test_be_netserver.c
	$(CC) $(CFLAGS) $^ -o $@ -lbiomeval
test_be_netclient: test_be_netclient.c
	$(CC) $(CFLAGS) $^ -o $@ -lbiomeval
test_construct_be_filerecstore: test_be_filerecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_filerecordstore: test_be_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_dbrecordstore: test_be_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ -lbiomeval
test_be_filerecordstore-stress: test_be_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DFILERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_dbrecordstore-stress: test_be_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DDBRECORDSTORETEST $^ -o $@ -lbiomeval
test_construct_be_archiverecstore: test_be_archiverecstore.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval
test_be_archiverecordstore: test_be_recordstore.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVEECORDSTORETEST $^ -o $@ -lbiomeval
test_be_archiverecordstore-stress: test_be_recordstore-stress.cpp
	$(CXX) $(CXXFLAGS) -DARCHIVERECORDSTORETEST $^ -o $@ -lbiomeval
test_be_timer: test_be_timer.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -lbiomeval

clean:
	$(RM) $(DISPOSABLEFILES) $(PROGS)
	$(RM) -r $(DISPOSABLEDIRS)
	$(RM) -rf *_test
