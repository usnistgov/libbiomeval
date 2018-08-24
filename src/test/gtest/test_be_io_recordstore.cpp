/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <be_io_utility.h>
#include <be_memory_autoarrayutility.h>

#include <gtest/gtest.h>

#ifdef FILERECORDSTORETEST
#include <be_io_filerecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef DBRECORDSTORETEST
#include <be_io_dbrecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef ARCHIVERECORDSTORETEST
#include <be_io_archiverecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef SQLITERECORDSTORETEST
#include <be_io_sqliterecstore.h>
#define TESTDEFINED
#define MERGETESTDEFINED
#endif

#ifdef COMPRESSEDRECORDSTORETEST
#include <be_io_compressedrecstore.h>
#include <be_io_dbrecstore.h>
#define TESTDEFINED
#endif

#ifdef TESTDEFINED
namespace BE = BiometricEvaluation;
#endif

static const int SEQUENCECOUNT = 10;
static const int RDATASIZE = 64;
static std::string rsname;

class ExistingRecordStore : public ::testing::Test {
protected:
	ExistingRecordStore()
	{
		EXPECT_NO_THROW(_rs = BE::IO::RecordStore::openRecordStore(
		    rsname));
		EXPECT_NE(_rs.get(), nullptr);
	}

	virtual ~ExistingRecordStore() {}

	std::shared_ptr<BE::IO::RecordStore> _rs;
};

static void
testSequence(
    std::shared_ptr<BE::IO::RecordStore> rs,
    std::vector<std::string> &description)
{
	try {
		for (int i = 0; ; i++) {
			try {
				auto entry = rs->sequence();
				description.push_back(entry.key + " - " +
				    to_string(entry.data));
			} catch (BE::Error::ObjectDoesNotExist) {
				break;
			} catch (BE::Error::Exception &e) {
				EXPECT_NO_THROW(throw e);
			}
		}
	} catch (BE::Error::Exception &e) {
		EXPECT_NO_THROW(throw e);
	}
}

#ifdef MERGETESTDEFINED
/*
 * Test the ability to merge RecordStores of different types
 */
static void
testMerge()
{
	BE::IO::RecordStore::Kind merged_type;
	const size_t num_rs = 3;
	const std::string merge_rs_fn[num_rs] = {"merge_test1", "merge_test2", 
	    "merge_test3"};
	BE::IO::RecordStore *merged_rs;
	BE::IO::RecordStore *merge_rs[num_rs];

#if defined ARCHIVERECORDSTORETEST
	merged_type = BE::IO::RecordStore::Kind::Archive;
	merge_rs[0] = new BE::IO::ArchiveRecordStore(merge_rs_fn[0],
	    "RS for merge");
	merge_rs[1] = new BE::IO::ArchiveRecordStore(merge_rs_fn[1],
	    "RS for merge");
	merge_rs[2] = new BE::IO::ArchiveRecordStore(merge_rs_fn[2],
	    "RS for merge");
#elif defined DBRECORDSTORETEST
	merged_type = BE::IO::RecordStore::Kind::BerkeleyDB;
	merge_rs[0] = new BE::IO::DBRecordStore(merge_rs_fn[0],
	    "RS for merge");
	merge_rs[1] = new BE::IO::DBRecordStore(merge_rs_fn[1],
	    "RS for merge");
	merge_rs[2] = new BE::IO::DBRecordStore(merge_rs_fn[2],
	    "RS for merge");
#elif defined FILERECORDSTORETEST
	merged_type = BE::IO::RecordStore::Kind::File;
	merge_rs[0] = new BE::IO::FileRecordStore(merge_rs_fn[0],
	    "RS for merge");
	merge_rs[1] = new BE::IO::FileRecordStore(merge_rs_fn[1],
	    "RS for merge");
	merge_rs[2] = new BE::IO::FileRecordStore(merge_rs_fn[2],
	    "RS for merge");
#elif defined SQLITERECORDSTORETEST
	merged_type = BE::IO::RecordStore::Kind::SQLite;
	merge_rs[0] = new BE::IO::SQLiteRecordStore(merge_rs_fn[0],
	    "RS for merge");
	merge_rs[1] = new BE::IO::SQLiteRecordStore(merge_rs_fn[1],
	    "RS for merge");
	merge_rs[2] = new BE::IO::SQLiteRecordStore(merge_rs_fn[2],
	    "RS for merge");
#else
	EXPECT_NO_THROW(throw BE::Error::StrategyBE::Error("Unknown "
	    "RecordStore type"));
#endif
	EXPECT_NO_THROW(merge_rs[0]->insert("0", "0", 2));
	EXPECT_NO_THROW(merge_rs[0]->insert("1", "1", 2));
	EXPECT_NO_THROW(merge_rs[0]->insert("2", "2", 2));
	EXPECT_NO_THROW(merge_rs[0]->sync());
	EXPECT_NO_THROW(merge_rs[1]->insert("3", "3", 2));
	EXPECT_NO_THROW(merge_rs[1]->insert("4", "4", 2));
	EXPECT_NO_THROW(merge_rs[1]->insert("5", "5", 2));
	EXPECT_NO_THROW(merge_rs[1]->sync());
	EXPECT_NO_THROW(merge_rs[2]->insert("6", "6", 2));
	EXPECT_NO_THROW(merge_rs[2]->insert("7", "7", 2));
	EXPECT_NO_THROW(merge_rs[2]->insert("8", "8", 2));
	EXPECT_NO_THROW(merge_rs[2]->sync());

	const std::string merged_rs_fn = "test_merged";
	std::vector<std::string> path;
	path.push_back(merge_rs_fn[0]);
	path.push_back(merge_rs_fn[1]);
	path.push_back(merge_rs_fn[2]);

	EXPECT_NO_THROW(BE::IO::RecordStore::mergeRecordStores(merged_rs_fn,
	    "A merge of 3 RS", merged_type, path));
#ifdef ARCHIVERECORDSTORETEST
	merged_rs = new BE::IO::ArchiveRecordStore(merged_rs_fn);
#elif defined DBRECORDSTORETEST
	merged_rs = new BE::IO::DBRecordStore(merged_rs_fn);
#elif defined FILERECORDSTORETEST
	merged_rs = new BE::IO::FileRecordStore(merged_rs_fn);
#elif defined SQLITERECORDSTORETEST
	merged_rs = new BE::IO::SQLiteRecordStore(merged_rs_fn);
#else
	EXPECT_NO_THROW(throw BE::Error::StrategyError("Unknown "
	    "RecordStore type"));
#endif
	EXPECT_NE(nullptr, merged_rs);
	ASSERT_EQ((num_rs * 3), merged_rs->getCount());

	/* Clean up */
	if (merged_rs != nullptr) {
		delete merged_rs; 
		EXPECT_NO_THROW(BE::IO::RecordStore::removeRecordStore(
		    merged_rs_fn));
	}
	for (uint64_t i = 0; i < num_rs; i++) {
		if (merge_rs[i] != nullptr) {
			delete merge_rs[i];
			EXPECT_NO_THROW(BE::IO::RecordStore::
			    removeRecordStore(merge_rs_fn[i]));
		}
	}
}
#endif

class NewRecordStore : public ::testing::Test {
protected:
	NewRecordStore()
	{
		std::string desc = "RW Test Dir";
#if defined FILERECORDSTORETEST
		EXPECT_NO_THROW(_rs.reset(
		    new BE::IO::FileRecordStore(rsname, desc)));
#elif defined DBRECORDSTORETEST
		EXPECT_NO_THROW(_rs.reset(
		    new BE::IO::DBRecordStore(rsname, desc)));
#elif defined ARCHIVERECORDSTORETEST
		EXPECT_NO_THROW(_rs.reset(
		    new BE::IO::ArchiveRecordStore(rsname, desc)));
#elif defined SQLITERECORDSTORETEST
		EXPECT_NO_THROW(_rs.reset(
		    new BE::IO::SQLiteRecordStore(rsname, desc)));
#elif defined COMPRESSEDRECORDSTORETEST
		EXPECT_NO_THROW(_rs.reset(
		    new BE::IO::CompressedRecordStore(rsname, desc,
	    	    BE::IO::RecordStore::Kind::BerkeleyDB, "GZIP")));
#else
		EXPECT_NO_THROW(_rs.reset(nullptr));
#endif
		
		EXPECT_NE(_rs.get(), nullptr);
	}

	virtual ~NewRecordStore() {}

#if defined FILERECORDSTORETEST
	std::shared_ptr<BE::IO::FileRecordStore> _rs;
#elif defined DBRECORDSTORETEST
	std::shared_ptr<BE::IO::DBRecordStore> _rs;
#elif defined ARCHIVERECORDSTORETEST
	std::shared_ptr<BE::IO::ArchiveRecordStore> _rs;
#elif defined SQLITERECORDSTORETEST
	std::shared_ptr<BE::IO::SQLiteRecordStore> _rs;
#elif defined COMPRESSEDRECORDSTORETEST
	std::shared_ptr<BE::IO::CompressedRecordStore> _rs;
#endif
};

/*
 * The tests.
 */

TEST_F(NewRecordStore, CRUD)
{
	std::string theKey = "firstRec";
	std::string wdata = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	uint64_t wlen = wdata.length();

	/* Insert one record */
	EXPECT_NO_THROW(this->_rs->insert(theKey, wdata.c_str(), wlen));
	EXPECT_EQ(1, this->_rs->getCount());

	/* Don't allow insertion of duplicate records */
	EXPECT_THROW(this->_rs->insert(theKey, wdata.c_str(), wlen),
	    BE::Error::ObjectExists);
	EXPECT_EQ(1, this->_rs->getCount());

	BE::Memory::uint8Array rdata{};
	
	/* Read test */
	EXPECT_NO_THROW(rdata = this->_rs->read(theKey));
	EXPECT_EQ(to_string(rdata), wdata);
	EXPECT_EQ(rdata.size(), wlen);
	EXPECT_EQ(1, this->_rs->getCount());

	/* Replace test */
	wdata = "ZYXWVUTSRQPONMLKJIHGFEDCBA0123456789";
        wlen = wdata.size();
	EXPECT_NO_THROW(this->_rs->replace(theKey, wdata.c_str(), wlen));
	EXPECT_EQ(1, this->_rs->getCount());

	/* Read replacement */
	EXPECT_NO_THROW(rdata = this->_rs->read(theKey));
	EXPECT_EQ(to_string(rdata), wdata);
	EXPECT_EQ(rdata.size(), wlen);
	EXPECT_EQ(1, this->_rs->getCount());

	/* Remove test */
	EXPECT_NO_THROW(this->_rs->remove(theKey));
	EXPECT_EQ(0, this->_rs->getCount());

	/* Read deleted record */
	EXPECT_THROW(rdata = this->_rs->read(theKey),
	    BE::Error::ObjectDoesNotExist);
}

TEST_F(ExistingRecordStore, sequence)
{
	/* Insert some data */
	std::string key;
	char rdata[RDATASIZE];
	for (int i = 0; i < SEQUENCECOUNT; i++) {
		bzero(rdata, RDATASIZE);
		snprintf(rdata, RDATASIZE, "key%u", i);
		key = rdata;
		snprintf(rdata, RDATASIZE, "%u", i);
		this->_rs->insert(key, rdata, RDATASIZE);
	}

	/* Sequence it back out */
	std::stringstream stream;
	std::vector<std::string> result, expectedResult;
	for (int i = 0; i < SEQUENCECOUNT; i++) {
		stream.str("");
		stream << "key" << i << " - " << i;
		expectedResult.push_back(stream.str());
	}
	testSequence(this->_rs, result);
	ASSERT_EQ(SEQUENCECOUNT, this->_rs->getCount());
	for (uint64_t i = 0; i < result.size(); i++)
		EXPECT_EQ(result[i], expectedResult[i]);

	/* Sequence into the middle, then test setCursorAtKey */
	auto entry = this->_rs->sequence(
	    BE::IO::RecordStore::BE_RECSTORE_SEQ_START);
	for (int i = 0; i < (SEQUENCECOUNT / 2); i++)
        	entry = this->_rs->sequence();
	key = "key3";
	EXPECT_NO_THROW(this->_rs->setCursorAtKey(key));
	result.clear();
	expectedResult.clear();
	for (int i = 3; i < SEQUENCECOUNT; i++) {
		stream.str("");
		stream << "key" << i << " - " << i;
		expectedResult.push_back(stream.str());
	}
	testSequence(this->_rs, result);
	ASSERT_EQ(expectedResult.size(), result.size());
	for (uint64_t i = 0; i < result.size(); i++)
		EXPECT_EQ(result[i], expectedResult[i]);

	/* Sequence from the end of a list */
	result.clear();
	expectedResult.clear();
	ASSERT_EQ(0, result.size());

	/* Sequence from a deleted key */
	key = "key3";
	EXPECT_NO_THROW(this->_rs->setCursorAtKey(key));
	for (int i = 4; i < SEQUENCECOUNT; i++) {
		stream.str("");
		stream << "key" << i << " - " << i;
		expectedResult.push_back(stream.str());
	}
	EXPECT_NO_THROW(this->_rs->remove(key));
	testSequence(this->_rs, result);
	ASSERT_EQ(result.size(), expectedResult.size());
	for (uint64_t i = 0; i < result.size(); i++)
		EXPECT_EQ(result[i], expectedResult[i]);

	/* Sequence from START */
	result.clear();
	expectedResult.clear();
	for (int i = 1; i <= SEQUENCECOUNT - 1; i++) {
		if (i == 3)
			continue;
		stream.str("");
		stream << "key" << i << " - " << i;
		expectedResult.push_back(stream.str());
	}
	EXPECT_NO_THROW(entry = this->_rs->sequence(
	    BE::IO::RecordStore::BE_RECSTORE_SEQ_START));
	testSequence(this->_rs, result);
	ASSERT_EQ(result.size(), expectedResult.size());
	for (uint64_t i = 0; i < result.size(); i++)
		EXPECT_EQ(result[i], expectedResult[i]);

	/* Reinsert removed key */
	snprintf(rdata, RDATASIZE, "%u", 3);
	EXPECT_NO_THROW(this->_rs->insert("key3", rdata, RDATASIZE));
	EXPECT_EQ(SEQUENCECOUNT, this->_rs->getCount());
}

TEST_F(ExistingRecordStore, zeroLengthChecks)
{
	std::string key = "key";
	char rdata[RDATASIZE];

	/* Write zero-length record */
	EXPECT_NO_THROW(this->_rs->insert(key, rdata, 0));

	/* Read zero-length record */
	BE::Memory::uint8Array data(100);
	EXPECT_NO_THROW(data = this->_rs->read(key));
	EXPECT_EQ(0, data.size());

	/* Remove zero-length record */
	EXPECT_NO_THROW(this->_rs->remove(key));
}

TEST_F(ExistingRecordStore, nonexistentChecks)
{
	ASSERT_EQ(0, this->_rs->getCount());
	std::string badKey = "lkdshfkjlshfkjhasjdfhlkasdf";

	/* Remove non-existent key */
	EXPECT_THROW(this->_rs->remove(badKey), BE::Error::ObjectDoesNotExist);

	/* Replace non-existent key */
	char rdata[RDATASIZE];
	EXPECT_THROW(this->_rs->replace(badKey, rdata, RDATASIZE),
	    BE::Error::ObjectDoesNotExist);

	/* Read non-existent key */
	EXPECT_THROW(auto data = this->_rs->read(badKey),
	    BE::Error::ObjectDoesNotExist);

	/* Length of non-existent key */
	EXPECT_THROW(this->_rs->length(badKey), BE::Error::ObjectDoesNotExist);

	/* Flush non-existent key */
	EXPECT_THROW(this->_rs->flush(badKey), BE::Error::ObjectDoesNotExist);

	/* Set cursor at non-existent key */
	EXPECT_THROW(this->_rs->setCursorAtKey(badKey),
	    BE::Error::ObjectDoesNotExist);
}

TEST_F(ExistingRecordStore, keyFormat)
{
	char rdata[RDATASIZE];

	EXPECT_THROW(this->_rs->insert("/Slash/", rdata, RDATASIZE),
	    BE::Error::StrategyError);
	EXPECT_THROW(this->_rs->insert("\\Back\\slash", rdata, RDATASIZE),
	    BE::Error::StrategyError);
	EXPECT_THROW(this->_rs->insert("*Asterisk*", rdata, RDATASIZE),
	    BE::Error::StrategyError);
	EXPECT_THROW(this->_rs->insert("&Ampersand&", rdata, RDATASIZE),
	    BE::Error::StrategyError);
}

#ifdef MERGETESTDEFINED
TEST(RecordStore, mergeRecordStores)
{
	//EXPECT_NO_THROW(testMerge());
	testMerge();
}
#endif /* MERGE_TEST */

#ifdef ARCHIVERECORDSTORETEST
TEST(ArchiveRecordStore, vacuum)
{
	BE::IO::ArchiveRecordStore *rs = new BE::IO::ArchiveRecordStore(rsname,
	    "");
	uint64_t startingSpace = rs->getSpaceUsed();
	EXPECT_NO_THROW(BE::IO::ArchiveRecordStore::vacuum(rsname));
	EXPECT_GE(startingSpace, rs->getSpaceUsed());
	delete rs;
}
#endif /* ARCHIVERECORDSTORETEST */

int
main(
    int argc,
    char *argv[])
{
	rsname = "rs_test";
	::testing::InitGoogleTest(&argc, argv);
	int retval = RUN_ALL_TESTS();
	
	EXPECT_NO_THROW(BE::IO::RecordStore::removeRecordStore(rsname));

	return (retval);
}

