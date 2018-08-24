/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

/*
 * Test the read and write operations of a RecordStore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract RecordStore by creating
 * an object of the appropriate implementation class.
 */

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#if defined FILERECORDSTORETEST
#include <be_io_filerecstore.h>
#elif defined DBRECORDSTORETEST
#include <be_io_dbrecstore.h>
#elif defined ARCHIVERECORDSTORETEST
#include <be_io_archiverecstore.h>
#elif defined SQLITERECORDSTORETEST
#include <be_io_sqliterecstore.h>
#endif

namespace BE = BiometricEvaluation;

//const uint64_t RECCOUNT = 1099997;	/* A prime number of records */
const uint64_t RECCOUNT = 110503;	/* A prime number of records */
const uint64_t RECSIZE = 1153;		/* of prime number size each */
//const int RECSIZE = 13859;		/* of prime number size each */
const uint64_t CREATEDESETROYCOUNT = 11;
const std::string rsname = "rs_test";

class StressRS : public ::testing::Test
{
protected:
	StressRS()
	{
		EXPECT_NO_THROW(_rs = BE::IO::RecordStore::openRecordStore(
		    rsname, BE::IO::Mode::ReadWrite));
		EXPECT_NE(_rs.get(), nullptr);
	}
	
	std::shared_ptr<BE::IO::RecordStore> _rs;

	virtual ~StressRS() = default;
};


TEST(RecordStoreStress, OpenClose)
{
	std::string descr = "RecordStore Stress Test";
	std::unique_ptr<BE::IO::RecordStore> rs;

	for (uint64_t i = 1; i <= CREATEDESETROYCOUNT; i++) {
#if defined FILERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::FileRecordStore(rsname, descr)));
#elif defined DBRECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::DBRecordStore(rsname, descr)));
#elif defined ARCHIVERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::ArchiveRecordStore(rsname, descr)));
#elif defined SQLITERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::SQLiteRecordStore(rsname, descr)));
#endif
		EXPECT_NE(nullptr, rs.get());
		rs.reset(nullptr);
		EXPECT_EQ(nullptr, rs.get());

		/* Test the re-open of an existing RecordStore. */
#if defined FILERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::FileRecordStore(rsname)));
#elif defined DBRECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::DBRecordStore(rsname)));
#elif defined ARCHIVERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::ArchiveRecordStore(rsname)));
#elif defined SQLITERECORDSTORETEST
		EXPECT_NO_THROW(
		    rs.reset(new BE::IO::SQLiteRecordStore(rsname)));
#endif
		EXPECT_NE(nullptr, rs.get());
		rs.reset(nullptr);
		EXPECT_EQ(nullptr, rs.get());

		/* Keep this RecordStore the last time */
		if (i != CREATEDESETROYCOUNT)
			EXPECT_NO_THROW(BE::IO::RecordStore::removeRecordStore(
			    rsname));
	}
}

TEST_F(StressRS, InsertMany)
{
	std::string theKey;
	uint8_t *theData = new (std::nothrow) uint8_t[RECSIZE];
	EXPECT_NE(nullptr, theData);

	for (uint64_t i = 0; i < RECCOUNT; i++) {
		theKey = "key" + std::to_string(i);
		EXPECT_NO_THROW(this->_rs->insert(theKey, theData, RECSIZE));
	}
	delete[] theData;
}

TEST_F(StressRS, ReplaceManySequential)
{
	std::string theKey;
	uint8_t *theData = new (std::nothrow) uint8_t[RECSIZE];
	EXPECT_NE(nullptr, theData);

	for (uint64_t i = 0; i < RECCOUNT; i++) {
		theKey = "key" + std::to_string(i);
		EXPECT_NO_THROW(this->_rs->replace(theKey, theData, RECSIZE));
	}
	delete[] theData;
}

TEST_F(StressRS, ReplaceManyRandom)
{
	std::string theKey;
	uint8_t *theData = new (std::nothrow) uint8_t[RECSIZE];
	EXPECT_NE(nullptr, theData);

	for (uint64_t i = 0; i < RECCOUNT; i++) {
		theKey = "key" + std::to_string(rand() % RECSIZE);
		EXPECT_NO_THROW(this->_rs->replace(theKey, theData, RECSIZE));
	}
	delete[] theData;
}

TEST_F(StressRS, ReadManySequential)
{
	std::string theKey;

	for (uint64_t i = 0; i < RECCOUNT; i++) {
		theKey = "key" + std::to_string(i);
		BE::Memory::uint8Array data;
		EXPECT_NO_THROW(data = this->_rs->read(theKey));
		EXPECT_EQ(data.size(), RECSIZE);

	}
}

TEST_F(StressRS, ReadManyRandom)
{
	std::string theKey;

	for (uint64_t i = 0; i < RECCOUNT; i++) {
		theKey = "key" + std::to_string(rand() % RECCOUNT);
		BE::Memory::uint8Array data;
		EXPECT_NO_THROW(data = this->_rs->read(theKey));
		EXPECT_EQ(data.size(), RECSIZE);
	}
}

TEST_F(StressRS, RemoveAll)
{
	for (uint64_t i = 0; i < RECCOUNT; i++)
		EXPECT_NO_THROW(this->_rs->remove("key" + std::to_string(i)));
}

int
main(
    int argc,
    char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	int retval = RUN_ALL_TESTS();

	EXPECT_NO_THROW(BE::IO::RecordStore::removeRecordStore(rsname));

	return (retval);
}

